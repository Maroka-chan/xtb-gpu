#include <iostream>
#include <fstream>
#include <vector>
#include <math.h>
#include <chrono>
#include <cblas.h>
#include <sycl/sycl.hpp>
using namespace sycl;
using namespace std::chrono;

// Row-major lower half matrix vector multiplication
void symv(
    const long N,
    const double alpha,
    const double *A,
    //const long lda,
    const double *X,
    //const long incX,
    const double beta,
    double *Y
    //const long incY
  ) {
  for (int i = 0; i < N; ++i) {
    double temp = 0.0;
    // Upper triangle (j >= i)
    for (int j = i; j < N; ++j) {
      temp += A[i * N + j] * X[j];
    }
    // Lower triangle (j < i)
    for (int j = 0; j < i; ++j) {
      temp += A[j * N + i] * X[j];
    }
    Y[i] = alpha * temp + beta * Y[i];
  }
}

double dot(
    const long N,
    const double *X,
    //const long incX,
    const double *Y
    //const long incY
  ) {
  double temp = 0.0;
  for (int i = 0; i < N; ++i) {
    temp += X[i] * Y[i];
  }
  return temp;
}

double get_third_order_energy(
    int qat_size,
    double *qat,
    int qsh_size,
    double *qsh,
    int atomicGam_size,
    double *atomicGam,
    int shellGam_size,
    double *shellGam
    ) {
  double energy = 0.0;

  if (atomicGam_size > 0) {
    for (int ii = 0; ii < qat_size; ii++) {
      energy += pow(qat[ii], 3) * atomicGam[ii] / 3.0;
    }
  }

  if (shellGam_size > 0) {
    for (int ii = 0; ii < qsh_size; ii++) {
      energy += pow(qsh[ii], 3) * shellGam[ii] / 3.0;
    }
  }

  return energy;
}

double get_isotropic_electrostatic_energy(
    int qat_size,
    double *qat,
    int qsh_size,
    double *qsh,
    int atomicGam_size,
    double *atomicGam,
    int shellGam_size,
    double *shellGam,
    int jmat_size,
    double *jmat_flat,
    int shift_size,
    double *shift
    ) {

  int n = jmat_size;
  symv(n, 1.0, jmat_flat, qsh, 0.0, shift);
  double eThird = get_third_order_energy(qat_size, qat, qsh_size, qsh, atomicGam_size, atomicGam, shellGam_size, shellGam);

  return 0.5 * dot(shift_size, shift, qsh) + eThird;
}

struct EnergyResults {
  double es;
  double scc;
};

EnergyResults electro(
    int nbf,
    double *H0,
    double *P,
    int dq_size,
    double *dq,
    int dqsh_size,
    double *dqsh,
    int atomicGam_size,
    double *atomicGam,
    int shellGam_size,
    double *shellGam,
    int jmat_size,
    double *jmat_flat,
    int shift_size,
    double *shift
    ) {

  int k = 0;
  double h = 0;
  for (int i = 0; i < nbf; i++) {
    for (int j = 0; j < i; j++) {
      h += P[i * nbf + j] * H0[k];
      k += 1;
    }
    h += P[i * nbf + i] * H0[k] * 0.5;
    k += 1;
  }

  double es = get_isotropic_electrostatic_energy(dq_size, dq, dqsh_size, dqsh, atomicGam_size, atomicGam, shellGam_size, shellGam, jmat_size, jmat_flat, shift_size, shift);

  double autoev = 27.21138505;
  double evtoau = 1.0 / autoev;
  double scc = es + 2.0 * h * evtoau;

  EnergyResults res;
  res.es = es;
  res.scc = scc;
  return res;
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

  // Flatten jmat
  std::vector<double> jmat_flat;
  for (const auto& row : jmat)
      jmat_flat.insert(jmat_flat.end(), row.begin(), row.end());


  queue q{gpu_selector_v};

  double* P_flat_dev = sycl::malloc_device<double>(P_flat.size(), q);
  q.memcpy(P_flat_dev, P_flat.data(), P_flat.size() * sizeof(double)).wait();

  double* H0_dev = sycl::malloc_device<double>(H0.size(), q);
  q.memcpy(H0_dev, H0.data(), H0.size() * sizeof(double)).wait();

  double* dq_dev = sycl::malloc_device<double>(dq.size(), q);
  q.memcpy(dq_dev, dq.data(), dq.size() * sizeof(double)).wait();

  double* dqsh_dev = sycl::malloc_device<double>(dqsh.size(), q);
  q.memcpy(dqsh_dev, dqsh.data(), dqsh.size() * sizeof(double)).wait();

  double* atomicGam_dev = sycl::malloc_device<double>(atomicGam.size(), q);
  q.memcpy(atomicGam_dev, atomicGam.data(), atomicGam.size() * sizeof(double)).wait();

  double* shellGam_dev = sycl::malloc_device<double>(shellGam.size(), q);
  q.memcpy(shellGam_dev, shellGam.data(), shellGam.size() * sizeof(double)).wait();

  double* jmat_flat_dev = sycl::malloc_device<double>(jmat_flat.size(), q);
  q.memcpy(jmat_flat_dev, jmat_flat.data(), jmat_flat.size() * sizeof(double)).wait();

  double* shift_dev = sycl::malloc_device<double>(shift.size(), q);
  q.memcpy(shift_dev, shift.data(), shift.size() * sizeof(double)).wait();

  EnergyResults* result_dev = sycl::malloc_shared<EnergyResults>(1, q);

  int dq_size = dq.size();
  int dqsh_size = dqsh.size();
  int atomicGam_size = atomicGam.size();
  int shellGam_size = shellGam.size();
  int jmat_size = jmat.size();
  int shift_size = shift.size();

  // Benchmark electro w/ molecule parallization
  auto t3 = high_resolution_clock::now();

  q.submit([&](sycl::handler& cgh) {
      cgh.parallel_for(sycl::range<1>(100000), [=](sycl::id<1> idx) {
        *result_dev = electro(nbf, H0_dev, P_flat_dev, dq_size, dq_dev, dqsh_size, dqsh_dev, atomicGam_size, atomicGam_dev, shellGam_size, shellGam_dev, jmat_size, jmat_flat_dev, shift_size, shift_dev);
      });
  });

  q.wait();

  auto t4 = high_resolution_clock::now();
  auto electro_par_duration = duration_cast<microseconds>(t4 - t3).count();
  std::cout << "electro parallel() time: " << electro_par_duration << " μs\n";


  // Benchmark electro
  auto t1 = high_resolution_clock::now();
  for (int i = 0; i < 100000; i++) {
    EnergyResults res = electro(nbf, H0.data(), P_flat.data(), dq.size(), dq.data(), dqsh.size(), dqsh.data(), atomicGam.size(), atomicGam.data(), shellGam.size(), shellGam.data(), jmat.size(), jmat_flat.data(), shift.size(), shift.data());
  }
  auto t2 = high_resolution_clock::now();
  auto electro_duration = duration_cast<microseconds>(t2 - t1).count();
  std::cout << "electro() time: " << electro_duration << " μs\n";


  // TODO: Free memory

  return 0;
}
