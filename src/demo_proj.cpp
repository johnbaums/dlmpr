// we only include RcppArmadillo.h which pulls Rcpp.h in for us
#include "RcppArmadillo.h"
using namespace Rcpp;
// [[Rcpp::depends(RcppArmadillo)]]

// [[Rcpp::export]]
NumericVector demographic_stochast(NumericVector v, NumericMatrix tmat)//, NumericMatrix stmat = NULL,  Logical fecundity1 = TRUE) 
{
  int tmncols = tmat.ncol();
  NumericMatrix sij(tmncols-1,tmncols);
  NumericVector sj(tmncols-1);
  NumericVector result(tmncols);
  int bs = sum(v);
  NumericVector Bi(bs);
  
  int b = 0;
  for(int i = 0; i<tmncols; i++) {
    if (tmat(0, i) > 0) 
      for(int ii = 0; ii<v[i]; ii++){
        Bi[ii+b] = R::rpois(tmat(0, i));
        //Rcpp::Rcout << ii << std::endl;
      }
      for (int j = 0; j<tmncols-1; j++) {
        if (tmat(j + 1, i) > 0) {
          if (tmat(j + 1, i) > 1) 
            sj[j] = sum(rpois(v[i], tmat(j + 1, i)));
          else sj[j] = sum(rbinom(v[i], 1, tmat(j + 1, i)));
        }
        else sj[j] = 0;
      }
      sij(_,i) = sj;
    b = b+v[i];
  }  
  result[0] = sum(Bi);
  for(int k = 0; k<tmncols-1;k++){
    result[k+1] = sum(sij(k,_));
  }
  return(result);
}

// [[Rcpp::export]]
NumericMatrix envir_stochast(NumericMatrix tmat, NumericMatrix sdmat, bool equalsign = true)
{ 
  arma::mat tmat1 = as<arma::mat>(tmat);
  arma::mat sdmat1 = as<arma::mat>(sdmat);
  arma::vec tmat_v = arma::vectorise(tmat1);
  arma::vec sdmat_v = arma::vectorise(sdmat1);
  int nvals = tmat_v.size(); 
  arma::vec mat_v = tmat_v.zeros();
  arma::vec deriv_v = tmat_v.zeros();
  int nc = tmat1.n_cols;
  int nr = tmat1.n_rows;     
  arma::mat mat1; 
  if (equalsign == false) {
    for(int i = 0; i<nvals; i++){
      mat_v[i] = R::rnorm(tmat_v[i], sdmat_v[i]);
    }
    mat1.insert_cols(0, mat_v);
    mat1.reshape(nr, nc);
  }     
  if (equalsign == true) {
    for(int i = 0; i<nvals; i++){
      Rcpp::RNGScope tmp;
      double draw = R::runif(-1,1);
      if (draw >= 0) {
        draw = 1;
      } else {
        draw = -1;
      }
      // Rcpp::Rcout << R::rnorm(0, sdmat_v[i])*draw << std::endl;
      deriv_v[i] = R::rnorm(0, sdmat_v[i])*draw;
    }                     
    deriv_v = abs(deriv_v);
    // Rcpp::Rcout << deriv_v << std::endl;
    mat1.insert_cols(0, deriv_v);
    mat1.reshape(nr, nc);
    // Rcpp::Rcout << mat1 << std::endl;
    mat1 = tmat1 + mat1;
    
  }       
  // mat[mat < 0] = 0;
  // mat[-1, ][mat[-1, ] > 1] = 1;
  return(wrap(mat1));
}

// [[Rcpp::export]]
NumericVector demo_proj(NumericVector v0, NumericMatrix tmat, Rcpp::Nullable<Rcpp::NumericMatrix> matsd= R_NilValue, 
                        Rcpp::Nullable<Rcpp::NumericMatrix> stmat = R_NilValue,
                        bool estamb=false, bool estdem=false, bool equalsign=true,  bool fecundity1=false) 
  {
    NumericVector v1 = v0;
    arma::vec av = as<arma::vec>(v0);
    arma::mat m1;
    m1.insert_cols(0, av);
    arma::mat tmat1 = as<arma::mat>(tmat);
    if (estamb == false && estdem == false)
      v1 = tmat1 * m1;
      wrap(v1);
      if (estamb == false && estdem == true && stmat.isNull() && fecundity1 == true) 
        v1 = demographic_stochast(v0, tmat);
        if (estamb == false && estdem == true && stmat.isNull() && fecundity1 == false)
          v1 =  demographic_stochast(v0, tmat);//, fecundity1 = false);
          if (estamb == false && estdem == true && stmat.isNotNull()) 
            v1 =  demographic_stochast(v0, tmat);//, stmat = stmat);
            if (estamb == true && estdem == false) {
              if (matsd.isNull()) 
                stop("there is not SD matrix provided\n (argument matsd=NULL)");
                //v1 = envir_stochast(tmat, matsd, equalsign = equalsign) * v0;
            }
//             if (estamb == true && estdem == true) {
//               if (matsd.isNotNull()) 
//                 stop("there is not SD matrix provided\n (argument matsd=NULL)");
//                 if (stmat.isNull() && fecundity1 == true) 
//                   //v1 =  demographic_stochast(v0, envir_stochast(tmat, matsd, equalsign = equalsign));
//                   //if (stmat.isNull() && fecundity1 == false); 
//                     //v1 =  demographic_stochast(v0, envir_stochast(tmat, matsd, equalsign = equalsign),fecundity1 = false);
//                     //if (stmat.isNotNull()) 
//                       //v1 =  demographic_stochast(v0, envir_stochast(tmat, matsd, equalsign = equalsign),stmat = stmat);
//             }
            return(v1);
}

// [[Rcpp::export]]
List demo_proj_n_cpp(List vn, NumericMatrix tmat, Rcpp::Nullable<Rcpp::NumericMatrix> matsd= R_NilValue, 
                       Rcpp::Nullable<Rcpp::NumericMatrix> stmat = R_NilValue,
                       bool estamb=false, bool estdem=false, bool equalsign=true,  bool fecundity1=false,
                       int nrep = 1, int time = 10)//, Rcpp::Nullable<Rcpp::NumericMatrix> management= R_NilValue, bool round = true) 
{
  List vn1 = vn;
  for (int i=0;i<time;i++) {
    for (int ii=0;ii<nrep;ii++) {
      NumericMatrix vii = vn[ii];
      NumericVector vii_i = vii(_,i+1);
//       Rcpp::Rcout << vii_i << std::endl;
      NumericVector v = demo_proj(vii_i, tmat, matsd, stmat, estamb, estdem,
                    equalsign, fecundity1);
//                    Rcpp::Rcout << v << std::endl;
        arma::vec v1 = as<arma::vec>(v);
        arma::mat m1 = as<arma::mat>(vn1[ii]);
//        Rcpp::Rcout << m1 << std::endl;
//        Rcpp::Rcout << v1 << std::endl;
        m1.insert_cols(m1.n_cols, v1);
        vn1[ii] =  m1; 
      }
  }
  return(vn1);
}

// You can include R code blocks in C++ files processed with sourceCpp
// (useful for testing and development). The R code will be automatically 
// run after the compilation.
//

 /*** R
# timesTwo(42)
*/