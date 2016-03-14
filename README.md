# Sandbox
Small test projects, experiments, and interesting bug reproductions

This project imports boost as a nuget package. If you clone the repo you'll need to enable refetching of that package on your local instance (the link to the package is already there in the solution files)

- StdUnorderedMapPerfIssues - a demonstration of perf issues in VC++ std::unordered_map, when hash values share low-order bits
  - Tested on VC14 and earlier; may have been fixed in later versions
