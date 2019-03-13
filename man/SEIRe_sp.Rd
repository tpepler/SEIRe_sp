\name{SEIRe_sp}
\alias{SEIRe_sp}
\title{Create a model for the SimInf framework}
\usage{
SEIRe_sp(ldata = NULL, gdata = NULL, u0 = NULL, tspan = NULL, events = NULL)
}
\arguments{
\item{ldata}{Data specific to each node in the model.
Can be specified either as a numeric matrix where column
\code{ldata[, j]} contains the local data vector for the
node \code{j} or as a \code{data.frame} with one row per
node.}
\item{gdata}{Data that are common to all nodes in the model.
Can be specified either as a named numeric vector or as a
one-row data.frame.}
\item{u0}{A \code{data.frame} with the initial state in each node.}
\item{tspan}{A vector (length >= 2) of increasing time points
where the state of each node is to be returned.}
\item{events}{A data.frame with scheduled events.}
}
\description{
Create a model to be used by the SimInf framework.
}
\examples{
## Please add example(s) how to use the model
}
