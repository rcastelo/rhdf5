\name{H5IdComponent-class}
\Rdversion{1.1}
\docType{class}
\alias{H5IdComponent-class}
\alias{H5IdComponent}
\alias{show,H5IdComponent-method}

\title{Class "H5IdComponent"}
\description{
A class representing a HDF5 identifier handle. HDF5 identifiers represent open files, groups, datasets, dataspaces, attributes, and datatypes. 
}
\section{Objects from the Class}{
Objects can be created by calls of \code{\link{H5Fcreate}}, \code{\link{H5Fopen}}, / \code{\link{H5Gcreate}}, \code{\link{H5Gopen}}, /  \code{\link{H5Dcreate}}, \code{\link{H5Dopen}}, \ \code{\link{H5Dget_space}}, \code{\link{H5Screate_simple}}, \ \code{\link{H5Acreate}}, \code{\link{H5Aopen}}.
}
\section{Slots}{
  \describe{
    \item{\code{ID}:}{Object of class \code{"integer"}. Contains the handle of C-type hid_t. }
  \item{native}{An object of class \code{logical}. If TRUE, array-like objects are treated as stored in HDF5 row-major rather than R column-major orientation. Using \code{native = TRUE} increases HDF5 file portability between programming languages. A file written with \code{native = TRUE} should also be read with \code{native = TRUE}}.
  }
}
\section{Methods}{
  \describe{
	\item{show}{\code{signature(object = "H5IdComponent")}: Shows the filename. }
    }
}
\references{\url{https://portal.hdfgroup.org/display/HDF5}}
\author{Bernd Fischer}
\seealso{
\link{rhdf5}
}
\examples{
showClass("H5IdComponent")
}
\keyword{classes}
