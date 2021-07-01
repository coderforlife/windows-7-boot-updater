:: Remove intermediate compilation files

@pushd %OUT%

del /F /Q *.obj *.netmodule *.resources *.res

@popd
