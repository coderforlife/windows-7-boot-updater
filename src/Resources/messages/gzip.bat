@IF ADDED_7ZIP_PATH==1 GOTO GZIP
@SET PATH=C:\Program Files\7-Zip;%PATH%
@SET ADDED_7ZIP_PATH=1
:GZIP
@7z a -bd -tgzip -mx9 -mfb258 -mpass15 %1.gz %1 >NUL
