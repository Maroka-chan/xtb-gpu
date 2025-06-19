#include <iostream>
#include <sycl/sycl.hpp>
#include <array>
#include <cmath>
using namespace sycl;

constexpr int maxElem = 86;
constexpr int carbon_n_shell = 2;

constexpr int saoshell_flat[24 * 5] = {
     0,  1,  0,  0,  0,
     4,  5,  0,  0,  0,
     8,  9,  0,  0,  0,
    12, 13,  0,  0,  0,
    16, 17,  0,  0,  0,
    20, 21,  0,  0,  0,
    24, 25,  0,  0,  0,
    28, 29,  0,  0,  0,
    32, 33,  0,  0,  0,
    36, 37,  0,  0,  0,
    40, 41,  0,  0,  0,
    44, 45,  0,  0,  0,
    48, 49,  0,  0,  0,
    52, 53,  0,  0,  0,
    56,  0,  0,  0,  0,
    57,  0,  0,  0,  0,
    58,  0,  0,  0,  0,
    59,  0,  0,  0,  0,
    60,  0,  0,  0,  0,
    61,  0,  0,  0,  0,
    62,  0,  0,  0,  0,
    63,  0,  0,  0,  0,
    64,  0,  0,  0,  0,
    65,  0,  0,  0,  0
};

constexpr double paulingEN[118] = {
    2.20,3.00, // H,He
    0.98,1.57,2.04,2.55,3.04,3.44,3.98,4.50, // Li-Ne
    0.93,1.31,1.61,1.90,2.19,2.58,3.16,3.50, // Na-Ar
    0.82,1.00, // K,Ca
    1.36,1.54,1.63,1.66,1.55, // Sc-
    1.83,1.88,1.91,1.90,1.65, // -Zn
    1.81,2.01,2.18,2.55,2.96,3.00, // Ga-Kr
    0.82,0.95, // Rb,Sr
    1.22,1.33,1.60,2.16,1.90, // Y-
    2.20,2.28,2.20,1.93,1.69, // -Cd
    1.78,1.96,2.05,2.10,2.66,2.60, // In-Xe
    0.79,0.89, // Cs,Ba
    1.10,1.12,1.13,1.14,1.15,1.17,1.18, // La-Eu
    1.20,1.21,1.22,1.23,1.24,1.25,1.26, // Gd-Yb
    1.27,1.30,1.50,2.36,1.90, // Lu-
    2.20,2.20,2.28,2.54,2.00, // -Hg
    1.62,2.33,2.02,2.00,2.20,2.20, // Tl-Rn
    // only dummies below
    1.50,1.50, // Fr,Ra
    1.50,1.50,1.50,1.50,1.50,1.50,1.50, // Ac-Am
    1.50,1.50,1.50,1.50,1.50,1.50,1.50, // Cm-No
    1.50,1.50,1.50,1.50,1.50, // Rf-
    1.50,1.50,1.50,1.50,1.50, // Rf-Cn
    1.50,1.50,1.50,1.50,1.50,1.50 // Nh-Og
};

constexpr int enScale_col_size = 4;
constexpr int enScale_size = enScale_col_size * enScale_col_size;
constexpr std::array<double, enScale_size> enScale_host = {
    0.02, 0.02, 0.02, 0.02,
    0.02, 0.02, 0.02, 0.02,
    0.02, 0.02, 0.02, 0.02,
    0.02, 0.02, 0.02, 0.02
};

constexpr double enScale4 = 0.0;

constexpr int kScale_col_size = 4;
constexpr int kScale_size = kScale_col_size * kScale_col_size;
constexpr std::array<double, kScale_size> kScale_host = {
    1.85, 2.04, 2.00, 2.04,
    2.04, 2.23, 2.00, 2.23,
    2.00, 2.00, 2.23, 2.23,
    2.04, 2.23, 2.23, 2.23
};

constexpr int valenceShell_col_size = carbon_n_shell;
constexpr int valenceShell_size = maxElem * valenceShell_col_size;
constexpr std::array<int, valenceShell_size> valenceShellValues = {{
    1,0,0, 1,1,0, 1,1,0, 1,1,0, 1,1,0, 1,1,0, 1,1,0, 1,1,0, 1,1,0, 1,1,1,
    1,1,0, 1,1,1, 1,1,1, 1,1,1, 1,1,1, 1,1,1, 1,1,1, 1,1,1, 1,1,0, 1,1,1,
    1,1,1, 1,1,1, 1,1,1, 1,1,1, 1,1,1, 1,1,1, 1,1,1, 1,1,1, 1,1,1, 1,1,0,
    1,1,1, 1,1,1, 1,1,1, 1,1,1, 1,1,1, 1,1,1, 1,1,0, 1,1,1, 1,1,1, 1,1,1,
    1,1,1, 1,1,1, 1,1,1, 1,1,1, 1,1,1, 1,1,1, 1,1,1, 1,1,1, 1,1,1, 1,1,1,
    1,1,1, 1,1,1, 1,1,0, 1,1,0, 1,1,0, 1,1,0, 1,1,0, 1,1,1, 1,1,1
}};

constexpr int angShell_col_size = 3;
constexpr int angShell_size = 86 * angShell_col_size;
constexpr std::array<int8_t, angShell_size> angShellValues = {{
   0, 0, 0,  0, 1, 0,  0, 1, 0,  0, 1, 0,  0, 1, 0,  0, 1, 0,  0, 1, 0,
   0, 1, 0,  0, 1, 0,  0, 1, 2,  0, 1, 0,  0, 1, 2,  0, 1, 2,  0, 1, 2,
   0, 1, 2,  0, 1, 2,  0, 1, 2,  0, 1, 2,  0, 1, 0,  0, 1, 2,  2, 0, 1,
   2, 0, 1,  2, 0, 1,  2, 0, 1,  2, 0, 1,  2, 0, 1,  2, 0, 1,  2, 0, 1,
   2, 0, 1,  0, 1, 0,  0, 1, 2,  0, 1, 2,  0, 1, 2,  0, 1, 2,  0, 1, 2,
   0, 1, 2,  0, 1, 0,  0, 1, 2,  2, 0, 1,  2, 0, 1,  2, 0, 1,  2, 0, 1,
   2, 0, 1,  2, 0, 1,  2, 0, 1,  2, 0, 1,  2, 0, 1,  0, 1, 0,  0, 1, 2,
   0, 1, 2,  0, 1, 2,  0, 1, 2,  0, 1, 2,  0, 1, 2,  0, 1, 0,  0, 1, 2,
   2, 0, 1,  2, 0, 1,  2, 0, 1,  2, 0, 1,  2, 0, 1,  2, 0, 1,  2, 0, 1,
   2, 0, 1,  2, 0, 1,  2, 0, 1,  2, 0, 1,  2, 0, 1,  2, 0, 1,  2, 0, 1,
   2, 0, 1,  2, 0, 1,  2, 0, 1,  2, 0, 1,  2, 0, 1,  2, 0, 1,  2, 0, 1,
   2, 0, 1,  2, 0, 1,  0, 1, 0,  0, 1, 0,  0, 1, 0,  0, 1, 0,  0, 1, 0,
   0, 1, 2,  0, 1, 2
}};

constexpr int atSize = 24;
constexpr std::array<int, atSize> atValues = {{
  6, 7, 6, 7, 6, 6, 6, 8, 7, 6, 8, 7, 6, 6, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
}};

// Atomic level information
constexpr int selfEnergy_col_size = 3;
constexpr int selfEnergy_size = selfEnergy_col_size * 86;
constexpr std::array<double, selfEnergy_size> selfEnergyValues = {
  -10.707211,  0.000000,  0.000000,
  -23.716445, -1.822307,  0.000000,
  -4.900000, -2.217789,  0.000000,
  -7.743081, -3.133433,  0.000000,
  -9.224376, -7.419002,  0.000000,
  -13.970922,-10.063292,  0.000000,
  -16.686243,-12.523956,  0.000000,
  -20.229985,-15.503117,  0.000000,
  -23.458179,-15.746583,  0.000000,
  -24.500000,-18.737298, -5.517827,
  -4.546934, -1.332719,  0.000000,
  -6.339908, -0.697688, -1.458197,
  -9.329017, -5.927846, -3.042325,
  -14.360932, -6.915131, -1.825036,
  -17.518756, -9.842286, -0.444893,
  -20.029654,-11.377694, -0.420282,
  -29.278781,-12.673758, -0.240338,
  -16.487730,-13.910539, -1.167213,
  -4.510348, -0.934377,  0.000000,
  -5.056506, -1.150304, -0.776883,
  -5.196187, -8.877940, -2.008206,
  -7.234331,-10.900000, -1.928783,
  -9.015342, -9.573347, -0.706647,
  -7.209794, -9.201304, -0.696957,
  -10.120933, -5.617346, -4.198724,
  -10.035473, -5.402911, -3.308988,
  -10.580430, -8.596723, -2.585753,
  -12.712236, -8.524281, -2.878873,
  -9.506548, -6.922958, -2.267723,
  -7.177294, -0.991895,  0.000000,
  -12.449656, -4.469873, -0.582255,
  -16.369792, -8.207673, -0.994226,
  -16.421504, -9.311147, -0.276830,
  -20.584732,-10.910799, -0.110636,
  -23.583718,-12.588824,  0.047980,
  -17.221422,-13.633377, -0.940657,
  -4.353793, -1.392938,  0.000000,
  -6.291692, -1.872475, -0.890492,
  -8.015206,-12.194181, -0.966195,
  -7.409832,-10.199105, -1.066939,
  -8.440821,-11.384021, -0.103760,
  -7.995133, -7.336245, -3.686225,
  -9.587897, -6.792444, -3.325525,
  -10.285405, -5.332608, -3.307153,
  -11.756644, -7.850495, -3.007906,
  -11.963518, -9.714059, -2.035281,
  -9.591083, -8.083960, -2.934333,
  -7.252341, -0.744865,  0.000000,
  -13.040909, -4.507143, -0.805666,
  -19.970428, -7.367059, -2.077548,
  -18.371244, -7.350148,  0.909033,
  -21.930653, -9.480374,  0.978922,
  -20.949407,-12.180159, -0.266596,
  -19.090498,-11.249471, -0.497097,
  -4.041706, -1.394193,  0.000000,
  -5.900000, -2.133395, -1.514900,
  -8.958783,-11.877410, -0.601717,
  -7.381991, -8.537781, -3.017508,
  -7.280875, -8.504806, -2.873159,
  -7.179760, -8.471830, -2.728809,
  -7.078644, -8.438855, -2.584460,
  -6.977529, -8.405879, -2.440110,
  -6.876413, -8.372904, -2.295761,
  -6.775298, -8.339929, -2.151411,
  -6.674182, -8.306953, -2.007062,
  -6.573067, -8.273978, -1.862712,
  -6.471951, -8.241003, -1.718363,
  -6.370836, -8.208027, -1.574013,
  -6.269720, -8.175052, -1.429664,
  -6.168604, -8.142076, -1.285314,
  -6.067489, -8.109101, -1.140965,
  -7.181755,-10.626891, -1.603430,
  -8.481353,-13.073088,  0.655254,
  -9.501505,-11.093016, -1.420389,
  -11.189119,-12.685198, -3.851981,
  -10.382841, -8.731460, -3.546379,
  -11.018475, -9.349164, -3.603762,
  -12.047728,-10.482306, -3.778297,
  -9.578599, -7.688552,  0.883399,
  -11.538066, -2.532581,  0.000000,
  -17.319333, -4.460584,  0.000000,
  -24.055207, -5.893816,  0.000000,
  -19.843840, -7.297456,  0.000000,
  -20.205380, -8.476927,  0.000000,
  -17.050229, -9.499822, -0.096063,
  -21.000000,-10.496406, -1.415056
};

// Exponent of the Slater function
constexpr int slaterExponent_col_size = 3;
constexpr int slaterExponent_size = slaterExponent_col_size * 86;
constexpr std::array<double, slaterExponent_size> slaterExponentValues = { // zeta_l
  1.230000,  0.000000,  0.000000,
  1.669667,  1.500000,  0.000000,
  0.750060,  0.557848,  0.000000,
  1.034720,  0.949332,  0.000000,
  1.479444,  1.479805,  0.000000,
  2.096432,  1.800000,  0.000000,
  2.339881,  2.014332,  0.000000,
  2.439742,  2.137023,  0.000000,
  2.416361,  2.308399,  0.000000,
  3.084104,  2.312051,  2.815609,
  0.763787,  0.573553,  0.000000,
  1.184203,  0.717769,  1.300000,
  1.352531,  1.391201,  1.000000,
  1.773917,  1.718996,  1.250000,
  1.816945,  1.903247,  1.167533,
  1.981333,  2.025643,  1.702555,
  2.485265,  2.199650,  2.476089,
  2.329679,  2.149419,  1.950531,
  0.875961,  0.631694,  0.000000,
  1.267130,  0.786247,  1.380000,
  2.440000,  1.358701,  1.019252,
  1.849994,  1.469983,  0.957410,
  1.673577,  1.383176,  0.938025,
  1.568211,  1.395427,  1.080270,
  1.839250,  1.222190,  1.240215,
  1.911049,  1.022393,  1.294467,
  2.326507,  1.464221,  1.298678,
  2.430756,  1.469945,  1.317046,
  2.375425,  1.550837,  1.984703,
  1.664847,  1.176434,  0.000000,
  1.720919,  1.591570,  1.050000,
  1.990429,  1.830340,  1.100000,
  2.026128,  1.949257,  1.040181,
  2.230969,  2.150656,  1.317549,
  2.077587,  2.263120,  1.845038,
  2.445680,  2.210494,  1.884991,
  1.017267,  0.870130,  0.000000,
  1.419028,  0.928932,  1.500000,
  2.670141,  1.633876,  1.165412,
  2.238668,  1.702480,  1.129590,
  1.706832,  1.666463,  1.132172,
  1.777658,  1.639917,  1.159781,
  1.918066,  1.918167,  1.346082,
  2.102697,  1.749643,  1.348322,
  2.458187,  1.811796,  1.398452,
  2.353691,  1.828354,  1.333352,
  2.843549,  1.798462,  1.266649,
  1.846689,  1.141823,  0.000000,
  1.963283,  1.685138,  1.050000,
  2.551510,  1.893784,  1.100000,
  2.307407,  2.179752,  1.256087,
  2.434144,  2.182459,  1.373076,
  2.159500,  2.308379,  1.691185,
  2.715140,  2.312510,  1.855707,
  1.225688,  0.823818,  0.000000,
  1.528102,  0.991572,  1.500000,
  2.875048,  1.731390,  1.303590,
  2.870000,  1.725197,  1.309804,
  2.872308,  1.729767,  1.315495,
  2.874615,  1.734337,  1.321186,
  2.876923,  1.738907,  1.326877,
  2.879231,  1.743478,  1.332567,
  2.881538,  1.748048,  1.338258,
  2.883846,  1.752618,  1.343949,
  2.886154,  1.757188,  1.349640,
  2.888462,  1.761758,  1.355331,
  2.890769,  1.766328,  1.361022,
  2.893077,  1.770899,  1.366713,
  2.895385,  1.775469,  1.372403,
  2.897692,  1.780039,  1.378094,
  2.900000,  1.784609,  1.383785,
  2.638729,  2.194333,  1.427467,
  2.018969,  1.996498,  1.407714,
  2.155885,  1.892022,  1.458186,
  2.262783,  2.187549,  1.636996,
  2.509631,  2.173991,  1.597888,
  2.756134,  2.117548,  1.680343,
  2.704492,  2.329136,  1.623286,
  3.241287,  2.183171,  2.084484,
  2.244504,  1.470848,  0.000000,
  2.294231,  1.731592,  0.000000,
  2.960592,  1.953130,  0.000000,
  2.788267,  2.277039,  0.000000,
  3.314810,  2.389456,  0.000000,
  2.220421,  2.408112,  1.500000,
  3.109394,  2.541934,  1.790000
};

//double* transform_s(const double* s, const double* trafo, sycl::queue& q) {
//  constexpr int N = 6;
//  double* s2 = sycl::malloc_shared<double>(N, q);
//
//  q.submit([&](sycl::handler& h) {
//    h.parallel_for(sycl::range<1>(N), [=](sycl::id<1> jj) {
//      double acc = 0.0;
//      for (int m = 0; m < N; ++m) {
//        acc += trafo[jj * N + m] * s[m];
//      }
//      s2[jj] = acc;
//    });
//  }).wait();
//
//  return s2;
//}

void dd(int nat, const int *saoshell, int saoshell_col_size) {
  // trans = np.zeros((1,3)) is always a vector of 3 zeros since we don't have a 3d infinite periodic boundary condition, i.e. we don't try to simulate an infinite grid of our molecule.  
  size_t trans_size = 1;

  size_t llaoMax = 7;

  size_t kmax = 3;
  
  size_t total_size = nat * nat * carbon_n_shell * carbon_n_shell * llaoMax * llaoMax * kmax;

  queue q{cpu_selector_v};

  std::cout << "Selected device: "
            << q.get_device().get_info<info::device::name>()
            << "\n";

  int nao = 5; // TODO: CHANGE THIS TO THE CORRECT VALUE !!!!!!!!!!!!!!
  int size = nao * (nao + 1) / 2;
  double* H0 = sycl::malloc_device<double>(size, q);

  int* electronegativity = sycl::malloc_shared<int>(maxElem, q);
  std::copy_n(paulingEN, maxElem, electronegativity);

  double* enScale = sycl::malloc_shared<double>(enScale_size, q);
  std::copy(enScale_host.begin(), enScale_host.end(), enScale);

  double* kScale = sycl::malloc_shared<double>(kScale_size, q);
  std::copy(kScale_host.begin(), kScale_host.end(), kScale);

  int pairParam_col_size = maxElem;
  double* pairParam = sycl::malloc_shared<double>(maxElem*maxElem, q);
  std::fill_n(pairParam, maxElem*maxElem, 1.0);

  int* valenceShell = sycl::malloc_shared<int>(valenceShell_size, q);
  std::copy_n(valenceShellValues.begin(), valenceShell_size, valenceShell);

  int8_t* angShell = sycl::malloc_shared<int8_t>(angShell_size, q);
  std::copy_n(angShellValues.begin(), angShell_size, angShell);

  int* at = sycl::malloc_shared<int>(atSize, q);
  std::copy_n(atValues.begin(), atSize, at);

  double* selfEnergy = sycl::malloc_shared<double>(selfEnergy_size, q);
  std::copy(selfEnergyValues.begin(), selfEnergyValues.end(), selfEnergy);

  double* slaterExponent = sycl::malloc_shared<double>(slaterExponent_size, q);
  std::copy(slaterExponentValues.begin(), slaterExponentValues.end(), slaterExponent);

  q.submit([&](sycl::handler& h) {
    h.parallel_for(sycl::range<1>(total_size), [=](sycl::id<1> tid) {
        size_t idx = tid[0];

        size_t iat  = idx / (llaoMax * llaoMax * trans_size * carbon_n_shell * carbon_n_shell * nat) % nat;
        size_t jat  = idx / (llaoMax * llaoMax * trans_size * carbon_n_shell * carbon_n_shell) % nat;
        size_t ish  = idx / (llaoMax * llaoMax * trans_size * carbon_n_shell) % carbon_n_shell;
        size_t jsh  = idx / (llaoMax * llaoMax * trans_size) % carbon_n_shell;
        size_t ii   = idx / llaoMax % llaoMax;
        size_t jj   = idx % llaoMax;

        int izp = at[iat];
        int jzp = at[jat];

        int hii = selfEnergy[iat * selfEnergy_col_size + ish];
        int hjj = selfEnergy[jat * selfEnergy_col_size + jsh];

        int iao = ii + saoshell[iat * saoshell_col_size + ish];
        int jao = jj + saoshell[jat * saoshell_col_size + jsh];

        int ij = std::min(iao, jao) + std::max(iao, jao) * (std::max(iao, jao) - 1) / 2;


        /////////// KM ////////////

        //h0scal(il, jl, izp, jzp, (valenceShell[izp, ish] != 0), (valenceShell[jzp, jsh] != 0))

        int ishtyp = angShell[izp * angShell_col_size + ish];
        int jshtyp = angShell[jzp * angShell_col_size + jsh];
        int il = ishtyp;
        int jl = jshtyp;

        bool valaoi = valenceShell[izp * valenceShell_col_size + ish] != 0;
        bool valaoj = valenceShell[jzp * valenceShell_col_size + jsh] != 0;
        double km = 0.0;
        if (valaoi && valaoj) {
          double den = std::pow(electronegativity[izp] - electronegativity[jzp], 2);
          double enpoly = (1.0 + enScale[il-1 * enScale_col_size + jl-1] * den * (1.0 + enScale4 * den));
          km = kScale[il-1 * kScale_col_size + jl-1] * enpoly * pairParam[jzp * pairParam_col_size + izp];
        }


        //# "DZ" functions (on H for GFN or 3S for EA calc on all atoms)
        //if (not valaoi and not valaoj):
        //    km = kdiff
        //    return km
        //if (not valaoi and valaoj):
        //    km = 0.5 * (kScale[jl-1, jl-1] + kdiff)
        //    return km
        //if (not valaoj and valaoi):
        //    km = 0.5 * (kScale[il-1, il-1] + kdiff)
        //return km

        ///////////////////////////

        double wExp = 0.5;
        double zi = slaterExponent[izp * slaterExponent_col_size + ish];
        double zj = slaterExponent[jzp * slaterExponent_col_size + jsh];
        double zetaij = std::pow(2 * std::sqrt(zi*zj) / (zi+zj), wExp); // Y term equation (7) in main.pdf
        double hav = 0.5 * km * (hii + hjj) * zetaij; // equation (1)

        H0[ij] += hav * shpoly * ss[ii, jj];

        std::cout << "jao: " << jao << "\n";

        //if (ii == 1) {
        //  std::cout << "ii: " << ii << ", jj: " << jj << "\n";
        //}

        //jao = jj + saoshell[jat * saoshell_size + jsh]
    });
  }).wait();

  sycl::free(H0, q);
  sycl::free(electronegativity, q);
  sycl::free(enScale, q);
  sycl::free(kScale, q);
  sycl::free(pairParam, q);
  sycl::free(valenceShell, q);
  sycl::free(angShell, q);
  sycl::free(at, q);
  sycl::free(selfEnergy, q);
  sycl::free(slaterExponent, q);
}

int main() {
  //queue q{gpu_selector_v};

  //std::cout << "Selected device: "
  //          << q.get_device().get_info<info::device::name>()
  //          << "\n";

  dd(24, saoshell_flat, 5);

  //for jj in range(6):
  //    sspher = 0
  //    for m in range(6):
  //        sspher = sspher + trafo[jj,m] * s[0,m]
  //    s2[0,jj] = sspher

  //constexpr int N = 6;
  //constexpr int SIZE = N * N;

  //double* trafo = sycl::malloc_shared<double>(SIZE, q);
  //std::copy_n(std::array<double, SIZE>{
  //  std::sqrt(1.0/5.0), std::sqrt(1.0/5.0), std::sqrt(1.0/5.0), 0.0, 0.0, 0.0,
  //  0.5*std::sqrt(3.0), -0.5*std::sqrt(3.0), 0.0, 0.0, 0.0, 0.0,
  //  0.5, 0.5, -1.0, 0.0, 0.0, 0.0,
  //  0.0, 0.0, 0.0, 1.0, 0.0, 0.0,
  //  0.0, 0.0, 0.0, 0.0, 1.0, 0.0,
  //  0.0, 0.0, 0.0, 0.0, 0.0, 1.0
  //}.begin(), SIZE, trafo);

  //double *s = sycl::malloc_shared<double>(6, q);
  //const std::array<double, SIZE> s_values = {
  //  3.75985585e-08, 0.0, 0.0, 0.0, 0.0, 0.0,
  //  0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
  //  0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
  //  0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
  //  0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
  //  0.0, 0.0, 0.0, 0.0, 0.0, 0.0
  //};
  //std::copy_n(s_values.begin(), SIZE, s);

  //double* s2 = transform_s(s, trafo, q);


  //float *s2    = sycl::malloc_shared<float>(6, q);

  //q.submit([&](sycl::handler &h) {
  //  h.parallel_for(sycl::range<1>(N), [=](sycl::id<1> jj) {
  //    float acc = 0.0f;
  //    acc += trafo[jj * N + 0] * s[0];
  //    acc += trafo[jj * N + 1] * s[1];
  //    acc += trafo[jj * N + 2] * s[2];
  //    acc += trafo[jj * N + 3] * s[3];
  //    acc += trafo[jj * N + 4] * s[4];
  //    acc += trafo[jj * N + 5] * s[5];

  //    s2[jj] = acc;
  //  });
  //}).wait();

  //for (int i = 0; i < N; ++i)
  //  std::cout << "s2[" << i << "] = " << s2[i] << "\n";

  //sycl::free(trafo, q);
  //sycl::free(s, q);
  //sycl::free(s2, q);
  return 0;
}
