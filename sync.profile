# Module dependencies.
# Every module that is required to build this module should have one entry.
# Each of the module version specifiers can take one of the following values:
#   - A specific Git revision.
#   - any git symbolic ref resolvable from the module's repository (e.g. "refs/heads/master" to track master branch)
#   - an empty string to use the same branch under test (dependencies will become "refs/heads/master" if we are in the master branch)
#
%dependencies = (
    "qttools" => "refs/heads/5.6",
    "qtx11extras" => "refs/heads/5.6",
    "qtsvg" => "refs/heads/5.6",
    "qtmultimedia" => "refs/heads/5.6",
    "qtdeclarative" => "refs/heads/5.6",
    "qtxmlpatterns" => "refs/heads/5.6",
    "qtbase" => "refs/heads/5.6",
    "qtwebview" => "refs/heads/5.6",
    "qtwebsockets" => "refs/heads/5.6",
);

