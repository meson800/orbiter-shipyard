$version = (git describe --tags --dirty)
$date = Get-Date -UFormat "%m-%d-%Y"

$version_cpp =  @"
#include "Version.h"
const char * Version::build_git_version = "$version";
const char * Version::build_date = "$date";
"@

$version_cpp > "Version.cpp"
