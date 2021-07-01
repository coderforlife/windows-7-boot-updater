:: Compress all langauages into their own gzip files along with creating a single zip file containing all of them

@FOR /F "tokens=*" %%A IN ('dir /b *.txt') DO CALL gzip %%A

@DEL messages.zip
7z a -tzip -mx9 -mfb258 -mpass15 messages.zip -i!*.txt >NUL
