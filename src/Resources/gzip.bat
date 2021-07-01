:: GZIP a file using 7zip
@set PATH=C:\Program Files\7-Zip;%PATH%
@7z a -bd -tgzip -mx9 -mfb258 -mpass15 %1.gz %1 >NUL
