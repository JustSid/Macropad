
add_library(fatfs STATIC ${SOURCES}
	fatfs/source/ff.c
	fatfs/source/ffsystem.c
	fatfs/source/ffunicode.c)
target_include_directories(fatfs INTERFACE fatfs/source)


add_library(tiny-json STATIC ${SOURCES}
	tiny-json/tiny-json.c)
target_include_directories(tiny-json INTERFACE tiny-json)
