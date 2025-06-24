#include <iostream>
#include <fstream>
#include <vector>
#include <math.h>
#include <chrono>
#include <cblas.h>
#include <sycl/sycl.hpp>
using namespace sycl;
using namespace std::chrono;

double autoev = 27.21138505;
double evtoau = 1.0 / autoev;

double get_third_order_energy(
    std::vector<double> qat,
    std::vector<double> qsh,
    std::vector<double> atomicGam,
    std::vector<double> shellGam
    ) {
  double energy = 0.0;

  if (!atomicGam.empty()) {
    for (int ii = 0; ii < qat.size(); ii++) {
      energy += pow(qat[ii], 3) * atomicGam[ii] / 3.0;
    }
  }

  if (!shellGam.empty()) {
    for (int ii = 0; ii < qsh.size(); ii++) {
      energy += pow(qsh[ii], 3) * shellGam[ii] / 3.0;
    }
  }

  return energy;
}

double get_isotropic_electrostatic_energy(
    std::vector<double> qat,
    std::vector<double> qsh,
    std::vector<double> atomicGam,
    std::vector<double> shellGam,
    std::vector<std::vector<double>> jmat,
    std::vector<double> shift
    ) {

  int n = jmat.size();

  // Ensure sizes match
  if (qsh.size() != n || shift.size() != n)
      throw std::runtime_error("Size mismatch in get_isotropic_electrostatic_energy");

  // Flatten jmat
  std::vector<double> jmat_flat;
  for (const auto& row : jmat)
      jmat_flat.insert(jmat_flat.end(), row.begin(), row.end());

  cblas_dsymv(CblasRowMajor, CblasLower, n, 1.0, jmat_flat.data(), n, qsh.data(), 1, 0.0, shift.data(), 1);

  double eThird = get_third_order_energy(qat, qsh, atomicGam, shellGam);

  return 0.5 * cblas_ddot(shift.size(), shift.data(), 1, qsh.data(), 1) + eThird;
}

std::tuple<double, double> electro(
    int nbf,
    std::vector<double> H0,
    std::vector<std::vector<double>> P,
    std::vector<double> dq,
    std::vector<double> dqsh,
    std::vector<double> atomicGam,
    std::vector<double> shellGam,
    std::vector<std::vector<double>> jmat,
    std::vector<double> shift
    ) {

  int k = 0;
  double h = 0;
  for (int i = 0; i < nbf; i++) {
    for (int j = 0; j < i; j++) {
      h += P[i][j] * H0[k];
      k += 1;
    }
    h += P[i][i] * H0[k] * 0.5;
    k += 1;
  }

  int iterations = nbf * (nbf + 1) / 2;
  std::cout << "iterations: " << iterations << "\n";

  double es = get_isotropic_electrostatic_energy(dq, dqsh, atomicGam, shellGam, jmat, shift);
  double scc = es + 2.0 * h * evtoau;

  return std::make_tuple(es, scc);
}

std::tuple<double, double> electro_sycl(
    int nbf,
    std::vector<double> H0,
    std::vector<double> P_flat,
    std::vector<double> dq,
    std::vector<double> dqsh,
    std::vector<double> atomicGam,
    std::vector<double> shellGam,
    std::vector<std::vector<double>> jmat,
    std::vector<double> shift
    ) {

  queue q{gpu_selector_v};

  //std::cout << "Selected device: "
  //          << q.get_device().get_info<info::device::name>()
  //          << "\n";


  size_t H0_size = H0.size();

  double* h_out = malloc_shared<double>(1, q);
  *h_out = 0.0;

  double* P_usm = sycl::malloc_shared<double>(nbf * nbf, q);
  double* H0_usm = sycl::malloc_shared<double>(H0_size, q);
  std::copy(P_flat.begin(), P_flat.end(), P_usm);
  std::copy(H0.begin(), H0.end(), H0_usm);

  q.submit([&](sycl::handler& cgh) {
      auto reduction = sycl::reduction(h_out, plus<>());

      cgh.parallel_for(sycl::range<1>(H0_size), reduction, [=](sycl::id<1> idx, auto& sum) {
          int k = idx[0];
          // Inverse triangular index calculation:
          int i = static_cast<int>((std::sqrt(8.0 * k + 1) - 1) / 2);
          int j = k - i * (i + 1) / 2;

          double val = P_usm[i * nbf + j] * H0_usm[k];
          if (i == j) val *= 0.5;

          sum += val;
      });
  });

  q.wait();

  double h = *h_out;
  free(h_out, q);
  free(P_usm, q);
  free(H0_usm, q);

  double es = get_isotropic_electrostatic_energy(dq, dqsh, atomicGam, shellGam, jmat, shift);
  double scc = es + 2.0 * h * evtoau;

  return std::make_tuple(es, scc);
}

int main() {
  int nbf = 66;

  std::ifstream file("data/H0.txt");
  std::vector<double> H0;
  double value;

  while (file >> value) {
      H0.push_back(value);
  }

  file.close();

  file.open("data/P.txt");
  std::vector<std::vector<double>> P;
  std::string line;

  while (std::getline(file, line)) {
      std::istringstream iss(line);
      std::vector<double> row;
      double val;
      while (iss >> val)
          row.push_back(val);
      P.push_back(row);
  }

  file.close();

  file.open("data/dq.txt");
  std::vector<double> dq;

  while (file >> value) {
      dq.push_back(value);
  }

  file.close();

  file.open("data/dqsh.txt");
  std::vector<double> dqsh;

  while (file >> value) {
      dqsh.push_back(value);
  }

  file.close();

  file.open("data/atomicGam.txt");
  std::vector<double> atomicGam;

  while (file >> value) {
      atomicGam.push_back(value);
  }

  file.close();

  file.open("data/shellGam.txt");
  std::vector<double> shellGam;

  while (file >> value) {
      shellGam.push_back(value);
  }

  file.close();

  file.open("data/jmat.txt");
  std::vector<std::vector<double>> jmat;

  while (std::getline(file, line)) {
      std::istringstream iss(line);
      std::vector<double> row;
      double val;
      while (iss >> val)
          row.push_back(val);
      jmat.push_back(row);
  }

  file.close();

  file.open("data/shift.txt");
  std::vector<double> shift;

  while (file >> value) {
      shift.push_back(value);
  }

  file.close();

  // Flatten P
  std::vector<double> P_flat;
  for (const auto& row : P)
      P_flat.insert(P_flat.end(), row.begin(), row.end());


  // Warm-up run to avoid one-time overhead in SYCL
  electro(nbf, H0, P, dq, dqsh, atomicGam, shellGam, jmat, shift);
  electro_sycl(nbf, H0, P_flat, dq, dqsh, atomicGam, shellGam, jmat, shift);

  // Benchmark electro
  auto t1 = high_resolution_clock::now();
  electro(nbf, H0, P, dq, dqsh, atomicGam, shellGam, jmat, shift);
  auto t2 = high_resolution_clock::now();
  auto electro_duration = duration_cast<microseconds>(t2 - t1).count();
  std::cout << "electro() time: " << electro_duration << " μs\n";

  // Benchmark electro_sycl
  auto t3 = high_resolution_clock::now();
  electro_sycl(nbf, H0, P_flat, dq, dqsh, atomicGam, shellGam, jmat, shift);
  auto t4 = high_resolution_clock::now();
  auto electro_sycl_duration = duration_cast<microseconds>(t4 - t3).count();
  std::cout << "electro_sycl() time: " << electro_sycl_duration << " μs\n";

  std::tuple<double, double> elec = electro(nbf, H0, P, dq, dqsh, atomicGam, shellGam, jmat, shift);
  double es = std::get<0>(elec);
  double scc = std::get<1>(elec);

  std::cout << "es: " << es << "\n";
  std::cout << "scc: " << scc << "\n";

  return 0;
}
