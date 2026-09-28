# Contributing to Percona Proxy

## Prerequisites

Basically, in order for us to be able to accept a contribution, it must be
licensed under [BSD-new](http://en.wikipedia.org/wiki/BSD_licenses). Upon
request, we can also provide a _contributor agreement_ for you to sign.

When you submit a pull request, add the following comment to your pull request.

> I am contributing the new code of the whole pull request, including one or
  several files that are either new files or modified ones, under the BSD-new
  license.

Without this comment, the pull request will not be accepted.

## Practicalities

* Please ensure that your pull-request has been made against the correct
  branch. For bug fixes or minor improvements, use the default branch (at the
  time of writing `2.1`).  For new features, use the `develop` branch.

* Please ensure that your code follows our [Coding Style](Development/coding-style-and-guidelines.md).
  All new code should be formatted with the
  [uncrustify configuration](uncrustify.cfg)
  provided with the Percona Proxy source code.
