#' sim_metapop
#'
#' Demographic projection
#' 
#' Make deterministic and stochastic demographic projections according to a transition matrix.
#' 
#'Simulate a metapopulation system in C++
#' this function is not complete.
#' @param nrep Number of simulations.
#' @param time Number of time steps
#' @param dist Distances between patches (symetrical matrix)
#' @param area Area of patches - This needs to be calculated somehow - using occupancy models?
#' @param presence Initial occupancies of patches. Must be presence 1 or absence 0.
#' @param y incidence function parameters
#' @param x incidence function parameters
#' @param e Minimum area of patches
#' @param alpha Exponential decay rate of patch connectivity (dispersion parameter)
#' @param beta double parameter that represents the shape of the dispersal kernel.
#' @param disp_fun char Which dispersal function to use. 'H' uses hanski(1994), if 'S' uses shaw(1995).
#' @param coln_fun char which colonisation function to use. 'H' = Hanski (1994), 'M'= Moilanen (2004), 'O'= Ovaskainen (2002). See decription for details.
#' @param locations NULL or NumericMatrix Longitudes and latitudes of coordinates of the patches
#' @param c double colonisation scale parameter see Ovaskainen 2002.
#' @export
#' @author Skipton Woolley
#' @seealso \code{link{metapop_n_cpp}}
#' @examples 
#' n <- 50
#' meta_data <- data.frame(x1=runif( n, min=-10, max=10), x2=runif( n, min=-10, max=10),
#' area=exp(-seq(.1,10,length.out = n))*10,presence=rbinom(n,1,.8))
#' area <- meta_data$area
#' dist <- as.matrix(with(meta_data, dist(cbind(x1, x2))))
#' presence <- meta_data$presence
#' locations <- meta_data[,c('x1','x2')]
#' mp <- sim_metapop(nrep=10, time=100, dist=dist, area=area, presence=presence,locations=locations,
#'                   x = 0.42, e = 0.0061, y = 1.2)

setGeneric("sim_metapop",
           function(nrep=10, time=20, dist, area, presence,
                    y = 1, x = 1, e=1, alpha = 1, beta = 1,
                    disp_fun = 'H',locations = NULL,
                    c=1,coln_fun='H') {
             # call c++ function that does this loop.
             mp <- dlmpr::metapop_n_cpp(nrep=nrep, time=time, dist=dist, area=area, presence=presence,
                                        y = y, x = x, e = e, alpha = alpha, beta = beta,
                                        disp_fun = disp_fun,locations = locations,
                                        c=c,coln_fun = coln_fun)
             sim_p_obs <- base::apply(plyr::ldply(base::lapply(mp,base::colSums),function(x)x),2,base::mean)
             sim_i_obs <- base::apply(plyr::ldply(base::lapply(base::lapply(mp,function(x)x[,-1]),function(x)base::rowSums(x)/time),function(x)x),2,base::mean)
             results <- base::list(mp = mp, sim_p_obs = sim_p_obs, sim_i_obs = sim_i_obs,
                             nrep=nrep, time = time, dist = dist, area = area, y = y, x = x, e = e, 
                             alpha = alpha, beta=beta,locations = locations)
             base::class(results) <- "sim_metapop"
             return(results)
           }
)