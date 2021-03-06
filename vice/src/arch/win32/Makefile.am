## Makefile.am for the Win32 version of `libarch.a'.

if !CROSS
SUBDIRS = evc mingw32 msvc utils vs_tmpl watcom
endif

SUFFIXES = .rc

AM_CPPFLAGS = \
	@ARCH_INCLUDES@ \
	@ARCH_INCLUDES@/msvc/wpcap \
	@ARCH_INCLUDES@/mingw32 \
	-I$(top_builddir)/src \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/src/drive \
	-I$(top_srcdir)/src/vdrive \
	-I$(top_srcdir)/src/monitor \
	-I$(top_srcdir)/src/rs232drv \
	-I$(top_srcdir)/src/c64 \
	-I$(top_srcdir)/src/c64dtv \
	-I$(top_srcdir)/src/c128 \
	-I$(top_srcdir)/src/vic20 \
	-I$(top_srcdir)/src/pet \
	-I$(top_srcdir)/src/plus4 \
	-I$(top_srcdir)/src/cbm2 \
	-I$(top_srcdir)/src/raster \
	-I$(top_srcdir)/src/userport \
	-I$(top_srcdir)/src/imagecontents \
	-I$(top_srcdir)/src/lib/p64 \
	-I$(top_srcdir)/src/platform \
	-I$(top_srcdir)/src/sid

noinst_LIBRARIES = libarch.a

libarch_a_SOURCES = \
	archdep.c \
	archdep.h \
	blockdev.c \
	c128ui.c \
	c64ui.c \
	c64dtvui.c \
	catweaselmkiii.c \
	cbm2ui.c \
	cbm5x0ui.c \
	console.c \
	dinput_handle.c \
	dinput_handle.h \
	dynlib.c \
	fullscrn.c \
	fullscrn.h \
	gifdrv.c \
	hardsid.c \
	intl.c \
	intl.h \
	joy.c \
	joy.h \
	kbd.c \
	kbd.h \
	lightpendrv.c \
	lightpendrv.h \
	mididrv.c \
	mousedrv.c \
	mousedrv.h \
	msvc-d3d.h \
	parsid.c \
	petui.c \
	plus4ui.c \
	res.rc \
	resacia.rc \
	resautostart.rc \
	resc128.rc \
	resc64.rc \
	resc64dtv.rc \
	resc64memoryhacks.rc \
	rescbm2common.rc \
	rescbm2.rc \
	rescbm5x0.rc \
	rescia.rc \
	rescommon.h \
	resdigimax.rc \
	resdqbb.rc \
	resdrivec128.rc \
	resdrivec64.rc \
	resdrivec64dtv.rc \
	resdrivepetcbm2.rc \
	resdriveplus4.rc \
	resdrivevic20.rc \
	resds12c887rtc.rc \
	reseasyflash.rc \
	resexpert.rc \
	resgeoram.rc \
	reside64.rc \
	resisepic.rc \
	reslightpen.rc \
	resmagicvoice.rc \
	resmidi.rc \
	resmmc64.rc \
	resmmcreplay.rc \
	resmouse.rc \
	respet.rc \
	respetcolour.rc \
	respetdww.rc \
	respetreu.rc \
	resplus4.rc \
	resramcart.rc \
	resretroreplay.rc \
	resreu.rc \
	resrs232user.rc \
	resscpu64.rc \
	ressid.rc \
	ressidcart.rc \
	ressiddtv.rc \
	ressoundexpander.rc \
	ressoundsampler.rc \
	rested.rc \
	restfe.rc \
	resuserportrtc.rc \
	resv364speech.rc \
	resvic.rc \
	resvic20.rc \
	resvicii.rc \
	resvsid.rc \
	rs232dev.c \
	scpu64ui.c \
	signals.c \
	socketimpl.h \
	statusbar.c \
	statusbar.h \
	rawnetarch.c \
	types.h \
	ui-resources.c \
	ui.c \
	ui.h \
	uiacia.c \
	uiacia.h \
	uiattach.c \
	uiattach.h \
	uiautostart.c \
	uiautostart.h \
	uic128.c \
	uic128.h \
	uic128model.c \
	uic128model.h \
	uic64burstmod.c \
	uic64burstmod.h \
	uic64cart.c \
	uic64cart.h \
	uic64dtv.c \
	uic64dtv.h \
	uic64dtvmodel.c \
	uic64dtvmodel.h \
	uic64memoryhacks.c \
	uic64memoryhacks.h \
	uic64model.c \
	uic64model.h \
	uicart.c \
	uicart.h \
	uicbm2model.c \
	uicbm2model.h \
	uicbm5x0model.c \
	uicbm5x0model.h \
	uicbm2set.c \
	uicbm2set.h \
	uicbm5x0set.c \
	uicbm5x0set.h \
	uicia.c \
	uicia.h \
	uicmdline.c \
	uiconsole.c \
	uidatasette.c \
	uidatasette.h \
	uidigimax.c \
	uidigimax.h \
	uidqbb.c \
	uidqbb.h \
	uidrivec64.c \
	uidrivec64.h \
	uidrivec64dtv.c \
	uidrivec64dtv.h \
	uidrivec128.c \
	uidrivec128.h \
	uidrivepetcbm2.c \
	uidrivepetcbm2.h \
	uidriveplus4.c \
	uidriveplus4.h \
	uidrivevic20.c \
	uidrivevic20.h \
	uids12c887rtc.c \
	uids12c887rtc.h \
	uieasyflash.c \
	uieasyflash.h \
	uievent.c \
	uievent.h \
	uiexpert.c \
	uiexpert.h \
	uifliplist.c \
	uifliplist.h \
	uigeoram.c \
	uigeoram.h \
	uihelp.c \
	uihelp.h \
	uiide64.c \
	uiide64.h \
	uiisepic.c \
	uiisepic.h \
	uijoystick.c \
	uijoystick.h \
	uikeyboard.c \
	uikeyboard.h \
	uilib.c \
	uilib.h \
	uilightpen.c \
	uilightpen.h \
	uimagicvoice.c \
	uimagicvoice.h \
	uimediafile.c \
	uimediafile.h \
	uimidi.c \
	uimidi.h \
	uimmc64.c \
	uimmc64.h \
	uimmcreplay.c \
	uimmcreplay.h \
	uimon.c \
	uimonmsg.h \
	uimouse.c \
	uimouse.h \
	uinetwork.c \
	uinetwork.h \
	uiperipheral.c \
	uiperipheral.h \
	uipetcolour.c \
	uipetcolour.h \
	uipetdww.c \
	uipetdww.h \
	uipetmodel.c \
	uipetmodel.h \
	uipetreu.c \
	uipetreu.h \
	uipetset.c \
	uipetset.h \
	uiplus4cart.c \
	uiplus4cart.h \
	uiplus4mem.c \
	uiplus4mem.h \
	uiplus4model.c \
	uiplus4model.h \
	uiquicksnapshot.c \
	uiquicksnapshot.h \
	uiram.c \
	uiram.h \
	uiramcart.c \
	uiramcart.h \
	uiretroreplay.c \
	uiretroreplay.h \
	uireu.c \
	uireu.h \
	uirom.c \
	uirom.h \
	uirs232.c \
	uirs232.h \
	uirs232user.c \
	uirs232user.h \
	uiscpu64.c \
	uiscpu64.h \
	uiscpu64model.c \
	uiscpu64model.h \
	uisid.c \
	uisid.h \
	uisidcart.c \
	uisidcart.h \
	uisiddtv.c \
	uisiddtv.h \
	uisnapshot.h \
	uisnapshot.c \
	uisound.c \
	uisound.h \
	uisoundexpander.c \
	uisoundexpander.h \
	uisoundsampler.c \
	uisoundsampler.h \
	uispeed.c \
	uispeed.h \
	uited.c \
	uited.h \
	uitfe.c \
	uitfe.h \
	uiuserportrtc.c \
	uiuserportrtc.h \
	uiv364speech.c \
	uiv364speech.h \
	uivic.c \
	uivic.h \
	uivic20model.c \
	uivic20model.h \
	uivicii.h \
	uivicii.c \
	uivicset.h \
	uivicset.c \
	uivideo.c \
	uivideo.h \
	vic20ui.c \
	video.c \
	video-ddraw.c \
	video-ddraw.h \
	video-dx9.c \
	videoarch.h \
	vsidui.c \
	vsyncarch.c \
	winjoy.h \
	winkbd.h \
	winlong.h \
	winmain.c \
	winmain.h

if OWCC
libarch_a_DEPENDENCIES = \
	resc128.res resc64.res resscpu64.res resc64dtv.res rescbm2.res rescbm5x0.res respet.res resplus4.res resvic20.res resvsid.res
else
libarch_a_DEPENDENCIES = \
	resc128.o resc64.o resscpu64.o resc64dtv.o rescbm2.o rescbm5x0.o respet.o resplus4.o resvic20.o resvsid.o
endif

EXTRA_DIST = \
	c64.ico \
	dirent.c \
	generate_msvc6_project.bat \
	generate_msvc70_project.bat \
	generate_msvc71_project.bat \
	generate_msvc8_project.bat \
	generate_msvc9_project.bat \
	generate_msvc10_project.bat \
	generate_msvc11_project.bat \
	generate_msvc12_project.bat \
	genmenuid.sh \
	genstringid.sh \
	genwinres.sh \
	make-bindist.sh \
	montb.bmp \
	res.txt \
	resdialogs.txt \
	resmenu.txt \
	resstrings.txt \
	system.c \
	vice.manifest \
	vice-version.bat \
	vice-version.bat.in

BUILT_SOURCES = res.h menuid.h stringid.h

if NATIVE_TOOLS
genwinres:	$(srcdir)/utils/genwinres.c
	$(NATIVE_TOOLS_COMPILER) -o genwinres $(srcdir)/utils/genwinres.c

res.h menuid.h stringid.h:	$(srcdir)/res.txt $(srcdir)/resdialogs.txt $(srcdir)/resmenu.txt $(srcdir)/resstrings.txt genwinres
	./genwinres res.h menuid.h stringid.h $(srcdir)/res.txt $(srcdir)/resdialogs.txt $(srcdir)/resmenu.txt $(srcdir)/resstrings.txt
else
resall.txt:	$(srcdir)/res.txt $(srcdir)/resdialogs.txt $(srcdir)/resmenu.txt $(srcdir)/resstrings.txt
	cat >$@ $^

res.h:	resall.txt
	echo "generating "$@
	. $(srcdir)/genwinres.sh <resall.txt >res.h

menuid.h:	resall.txt
	echo "generating "$@
	. $(srcdir)/genmenuid.sh <resall.txt >menuid.h

stringid.h:	resall.txt
	echo "generating "$@
	. $(srcdir)/genstringid.sh <resall.txt >stringid.h
endif

resc128cat.rc: $(srcdir)/resc128.rc $(srcdir)/res.rc res.h \
	$(srcdir)/resacia.rc $(srcdir)/resrs232user.rc $(srcdir)/ressid.rc \
	$(srcdir)/resvicii.rc $(srcdir)/resreu.rc $(srcdir)/resgeoram.rc \
	$(srcdir)/resramcart.rc $(srcdir)/resmmc64.rc $(srcdir)/reside64.rc \
	$(srcdir)/resdrivec128.rc $(srcdir)/resdigimax.rc \
	$(srcdir)/resmouse.rc $(srcdir)/restfe.rc $(srcdir)/resmidi.rc \
	$(srcdir)/resdqbb.rc $(srcdir)/resisepic.rc \
	$(srcdir)/reslightpen.rc $(srcdir)/ressoundexpander.rc \
	$(srcdir)/reseasyflash.rc $(srcdir)/resmagicvoice.rc \
	$(srcdir)/resexpert.rc $(srcdir)/resautostart.rc \
	$(srcdir)/resmmcreplay.rc $(srcdir)/resretroreplay.rc \
	$(srcdir)/rescia.rc $(srcdir)/resds12c887rtc.rc \
	$(srcdir)/resuserportrtc.rc $(top_builddir)/src/debug.h
	cat >resc128cat.rc $(srcdir)/resc128.rc $(srcdir)/res.rc \
	$(srcdir)/resacia.rc $(srcdir)/reside64.rc $(srcdir)/resreu.rc \
	$(srcdir)/resgeoram.rc $(srcdir)/resramcart.rc $(srcdir)/resmmc64.rc \
	$(srcdir)/resrs232user.rc $(srcdir)/ressid.rc $(srcdir)/resvicii.rc \
	$(srcdir)/resdrivec128.rc $(srcdir)/resdigimax.rc \
	$(srcdir)/resmouse.rc $(srcdir)/restfe.rc $(srcdir)/reslightpen.rc \
	$(srcdir)/ressoundexpander.rc $(srcdir)/reseasyflash.rc \
	$(srcdir)/resmagicvoice.rc $(srcdir)/resexpert.rc \
	$(srcdir)/resautostart.rc $(srcdir)/resmidi.rc $(srcdir)/resdqbb.rc $(srcdir)/resisepic.rc \
	$(srcdir)/resmmcreplay.rc $(srcdir)/resretroreplay.rc \
	$(srcdir)/rescia.rc $(srcdir)/resds12c887rtc.rc \
	$(srcdir)/resuserportrtc.rc

resc64cat.rc: $(srcdir)/resc64.rc $(srcdir)/res.rc res.h $(srcdir)/resacia.rc \
	$(srcdir)/resrs232user.rc $(srcdir)/ressid.rc $(srcdir)/resvicii.rc \
	$(srcdir)/resreu.rc $(srcdir)/resgeoram.rc $(srcdir)/resramcart.rc \
	$(srcdir)/resmmc64.rc $(srcdir)/resc64memoryhacks.rc \
	$(srcdir)/reside64.rc $(srcdir)/resautostart.rc \
	$(srcdir)/resdrivec64.rc $(srcdir)/resdigimax.rc $(srcdir)/resmouse.rc \
	$(srcdir)/restfe.rc $(srcdir)/resmidi.rc $(srcdir)/resdqbb.rc \
	$(srcdir)/resisepic.rc $(srcdir)/reslightpen.rc \
	$(srcdir)/ressoundexpander.rc $(srcdir)/reseasyflash.rc \
	$(srcdir)/resmagicvoice.rc $(srcdir)/resexpert.rc \
	$(srcdir)/resmmcreplay.rc $(srcdir)/resretroreplay.rc \
	$(srcdir)/rescia.rc $(srcdir)/resds12c887rtc.rc \
	$(srcdir)/resuserportrtc.rc $(top_builddir)/src/debug.h
	cat >resc64cat.rc $(srcdir)/resc64.rc $(srcdir)/res.rc \
	$(srcdir)/resacia.rc $(srcdir)/resdrivec64.rc \
	$(srcdir)/reside64.rc $(srcdir)/resreu.rc $(srcdir)/resgeoram.rc \
	$(srcdir)/resmmc64.rc $(srcdir)/resramcart.rc \
	$(srcdir)/resc64memoryhacks.rc $(srcdir)/resrs232user.rc \
	$(srcdir)/ressid.rc $(srcdir)/resvicii.rc $(srcdir)/resdigimax.rc \
	$(srcdir)/resmouse.rc $(srcdir)/restfe.rc $(srcdir)/resmidi.rc \
	$(srcdir)/resdqbb.rc $(srcdir)/resisepic.rc $(srcdir)/reslightpen.rc \
	$(srcdir)/reseasyflash.rc $(srcdir)/resmagicvoice.rc \
	$(srcdir)/resexpert.rc $(srcdir)/ressoundexpander.rc \
	$(srcdir)/resautostart.rc $(srcdir)/resmmcreplay.rc \
	$(srcdir)/resretroreplay.rc $(srcdir)/rescia.rc \
	$(srcdir)/resds12c887rtc.rc $(srcdir)/resuserportrtc.rc

resscpu64cat.rc: $(srcdir)/resscpu64.rc $(srcdir)/res.rc res.h $(srcdir)/resacia.rc \
	$(srcdir)/resrs232user.rc $(srcdir)/ressid.rc $(srcdir)/resvicii.rc \
	$(srcdir)/resreu.rc $(srcdir)/resgeoram.rc $(srcdir)/resramcart.rc \
	$(srcdir)/resmmc64.rc $(srcdir)/reside64.rc \
	$(srcdir)/resautostart.rc $(srcdir)/resdrivec64.rc \
	$(srcdir)/resdigimax.rc $(srcdir)/resmouse.rc \
	$(srcdir)/restfe.rc $(srcdir)/resmidi.rc $(srcdir)/resdqbb.rc \
	$(srcdir)/resisepic.rc $(srcdir)/reslightpen.rc \
	$(srcdir)/ressoundexpander.rc $(srcdir)/reseasyflash.rc \
	$(srcdir)/resmagicvoice.rc $(srcdir)/resexpert.rc \
	$(srcdir)/resmmcreplay.rc $(srcdir)/resretroreplay.rc \
	$(srcdir)/rescia.rc $(srcdir)/resds12c887rtc.rc \
	$(srcdir)/resuserportrtc.rc $(top_builddir)/src/debug.h
	cat >resscpu64cat.rc $(srcdir)/resscpu64.rc $(srcdir)/res.rc \
	$(srcdir)/resacia.rc $(srcdir)/resdrivec64.rc \
	$(srcdir)/reside64.rc $(srcdir)/resreu.rc $(srcdir)/resgeoram.rc \
	$(srcdir)/resmmc64.rc $(srcdir)/resramcart.rc \
	$(srcdir)/resrs232user.rc $(srcdir)/ressid.rc $(srcdir)/resvicii.rc \
	$(srcdir)/resdigimax.rc $(srcdir)/resmouse.rc $(srcdir)/restfe.rc \
	$(srcdir)/resmidi.rc $(srcdir)/resdqbb.rc $(srcdir)/resisepic.rc \
	$(srcdir)/reslightpen.rc $(srcdir)/reseasyflash.rc \
	$(srcdir)/resmagicvoice.rc $(srcdir)/resexpert.rc \
	$(srcdir)/ressoundexpander.rc $(srcdir)/resautostart.rc \
	$(srcdir)/resmmcreplay.rc $(srcdir)/resretroreplay.rc \
	$(srcdir)/rescia.rc $(srcdir)/resds12c887rtc.rc \
	$(srcdir)/resuserportrtc.rc

resc64dtvcat.rc : $(srcdir)/resc64dtv.rc $(srcdir)/res.rc res.h \
	$(srcdir)/ressiddtv.rc $(srcdir)/resvicii.rc \
	$(srcdir)/resdrivec64dtv.rc $(srcdir)/resautostart.rc \
	$(top_builddir)/src/debug.h
	cat >resc64dtvcat.rc $(srcdir)/resc64dtv.rc $(srcdir)/res.rc \
	$(srcdir)/resdrivec64dtv.rc $(srcdir)/ressiddtv.rc \
	$(srcdir)/resautostart.rc $(srcdir)/resvicii.rc

rescbm2cat.rc: $(srcdir)/rescbm2.rc $(srcdir)/rescbm2common.rc \
	$(srcdir)/res.rc res.h $(srcdir)/resacia.rc $(srcdir)/ressiddtv.rc \
	$(srcdir)/resdrivepetcbm2.rc $(srcdir)/resautostart.rc \
	$(srcdir)/rescia.rc $(top_builddir)/src/debug.h
	cat >rescbm2cat.rc $(srcdir)/rescbm2.rc $(srcdir)/rescbm2common.rc \
	$(srcdir)/res.rc $(srcdir)/resacia.rc $(srcdir)/resdrivepetcbm2.rc \
	$(srcdir)/ressiddtv.rc $(srcdir)/resautostart.rc \
	$(srcdir)/rescia.rc

rescbm5x0cat.rc: $(srcdir)/rescbm5x0.rc $(srcdir)/rescbm2common.rc \
	$(srcdir)/res.rc res.h $(srcdir)/resacia.rc $(srcdir)/ressiddtv.rc \
	$(srcdir)/resdrivepetcbm2.rc $(srcdir)/resautostart.rc \
	$(srcdir)/resvicii.rc $(srcdir)/rescia.rc $(top_builddir)/src/debug.h
	cat >rescbm5x0cat.rc $(srcdir)/rescbm5x0.rc $(srcdir)/rescbm2common.rc \
	$(srcdir)/res.rc $(srcdir)/resacia.rc $(srcdir)/resdrivepetcbm2.rc \
	$(srcdir)/ressiddtv.rc $(srcdir)/resvicii.rc $(srcdir)/resautostart.rc \
	$(srcdir)/rescia.rc

respetcat.rc: $(srcdir)/respet.rc $(srcdir)/res.rc res.h $(srcdir)/resacia.rc \
	$(srcdir)/resdrivepetcbm2.rc $(srcdir)/respetreu.rc \
	$(srcdir)/resautostart.rc $(srcdir)/ressidcart.rc \
	$(srcdir)/respetdww.rc $(srcdir)/respetcolour.rc \
	$(top_builddir)/src/debug.h
	cat >respetcat.rc $(srcdir)/respet.rc $(srcdir)/res.rc \
	$(srcdir)/resacia.rc $(srcdir)/resdrivepetcbm2.rc \
	$(srcdir)/respetreu.rc $(srcdir)/ressidcart.rc \
	$(srcdir)/respetdww.rc $(srcdir)/respetcolour.rc \
	$(srcdir)/resautostart.rc

resplus4cat.rc: $(srcdir)/resplus4.rc $(srcdir)/res.rc res.h \
	$(srcdir)/resacia.rc $(srcdir)/resdriveplus4.rc \
	$(srcdir)/resautostart.rc $(srcdir)/ressidcart.rc \
	$(srcdir)/resv364speech.rc $(srcdir)/rested.rc \
	$(top_builddir)/src/debug.h
	cat >resplus4cat.rc $(srcdir)/resplus4.rc $(srcdir)/res.rc \
	$(srcdir)/resacia.rc $(srcdir)/resdriveplus4.rc \
	$(srcdir)/resv364speech.rc \
	$(srcdir)/ressidcart.rc $(srcdir)/resautostart.rc \
	$(srcdir)/rested.rc

resvic20cat.rc : $(srcdir)/resvic20.rc $(srcdir)/res.rc res.h \
	$(srcdir)/resrs232user.rc $(srcdir)/resdrivevic20.rc \
	$(srcdir)/ressidcart.rc $(srcdir)/resmidi.rc \
	$(srcdir)/resautostart.rc $(srcdir)/reslightpen.rc \
	$(srcdir)/resacia.rc $(srcdir)/resgeoram.rc \
	$(srcdir)/resdigimax.rc $(srcdir)/restfe.rc \
	$(srcdir)/ressoundexpander.rc $(srcdir)/ressoundsampler.rc \
	$(srcdir)/resvic.rc $(srcdir)/resds12c887rtc.rc \
	$(top_builddir)/src/debug.h
	cat >resvic20cat.rc $(srcdir)/resvic20.rc $(srcdir)/res.rc \
	$(srcdir)/resdrivevic20.rc $(srcdir)/resrs232user.rc \
	$(srcdir)/ressidcart.rc $(srcdir)/resmidi.rc \
	$(srcdir)/reslightpen.rc $(srcdir)/resautostart.rc \
	$(srcdir)/resacia.rc $(srcdir)/resgeoram.rc \
	$(srcdir)/resdigimax.rc $(srcdir)/restfe.rc \
	$(srcdir)/ressoundexpander.rc $(srcdir)/ressoundsampler.rc \
	$(srcdir)/resvic.rc $(srcdir)/resds12c887rtc.rc

resvsidcat.rc: $(srcdir)/resvsid.rc $(srcdir)/res.rc res.h $(srcdir)/ressid.rc \
	$(srcdir)/resvicii.rc $(top_builddir)/src/debug.h
	cat >resvsidcat.rc $(srcdir)/resvsid.rc $(srcdir)/res.rc \
	$(srcdir)/ressid.rc $(srcdir)/resvicii.rc

if OWCC
resc128.res: resc128cat.rc
	wrc -fo=resc128.res -r -bt=nt \
	-i=$(top_builddir)/src \
	-i=$(top_srcdir)/src/arch/win32 \
	-i=$(top_srcdir)/src \
	resc128cat.rc
else
resc128.o: resc128cat.rc
	$(WRC) -i resc128cat.rc -o resc128.res \
	--include-dir $(top_builddir)/src \
	--include-dir $(top_srcdir)/src/arch/win32 \
	--include-dir $(top_srcdir)/src
	$(WINDRES) -i resc128.res -o $@
endif

if OWCC
resc64.res: resc64cat.rc
	wrc -fo=resc64.res -r -bt=nt \
	-i=$(top_builddir)/src \
	-i=$(top_srcdir)/src/arch/win32 \
	-i=$(top_srcdir)/src \
	resc64cat.rc
else
resc64.o: resc64cat.rc
	$(WRC) -i resc64cat.rc -o resc64.res \
	--include-dir $(top_builddir)/src \
	--include-dir $(top_srcdir)/src/arch/win32  \
	--include-dir $(top_srcdir)/src
	$(WINDRES) -i resc64.res -o $@
endif

if OWCC
resscpu64.res: resscpu64cat.rc
	wrc -fo=resscpu64.res -r -bt=nt \
	-i=$(top_builddir)/src \
	-i=$(top_srcdir)/src/arch/win32 \
	-i=$(top_srcdir)/src \
	resscpu64cat.rc
else
resscpu64.o: resscpu64cat.rc
	$(WRC) -i resscpu64cat.rc -o resscpu64.res \
	--include-dir $(top_builddir)/src \
	--include-dir $(top_srcdir)/src/arch/win32  \
	--include-dir $(top_srcdir)/src
	$(WINDRES) -i resscpu64.res -o $@
endif

if OWCC
resc64dtv.res: resc64dtvcat.rc
	wrc -fo=resc64dtv.res -r -bt=nt \
	-i=$(top_builddir)/src \
	-i=$(top_srcdir)/src/arch/win32 \
	-i=$(top_srcdir)/src \
	resc64dtvcat.rc
else
resc64dtv.o: resc64dtvcat.rc
	$(WRC) -i resc64dtvcat.rc -o resc64dtv.res \
	--include-dir $(top_builddir)/src \
	--include-dir $(top_srcdir)/src/arch/win32  \
	--include-dir $(top_srcdir)/src
	$(WINDRES) -i resc64dtv.res -o $@
endif

if OWCC
rescbm2.res: rescbm2cat.rc
	wrc -fo=rescbm2.res -r -bt=nt \
	-i=$(top_builddir)/src \
	-i=$(top_srcdir)/src/arch/win32 \
	-i=$(top_srcdir)/src \
	rescbm2cat.rc
else
rescbm2.o: rescbm2cat.rc
	$(WRC) -i rescbm2cat.rc -o rescbm2.res \
	--include-dir $(top_builddir)/src \
	--include-dir $(top_srcdir)/src/arch/win32 \
	--include-dir $(top_srcdir)/src
	$(WINDRES) -i rescbm2.res -o $@
endif

if OWCC
rescbm5x0.res: rescbm5x0cat.rc
	wrc -fo=rescbm5x0.res -r -bt=nt \
	-i=$(top_builddir)/src \
	-i=$(top_srcdir)/src/arch/win32 \
	-i=$(top_srcdir)/src \
	rescbm5x0cat.rc
else
rescbm5x0.o: rescbm5x0cat.rc
	$(WRC) -i rescbm5x0cat.rc -o rescbm5x0.res \
	--include-dir $(top_builddir)/src \
	--include-dir $(top_srcdir)/src/arch/win32 \
	--include-dir $(top_srcdir)/src
	$(WINDRES) -i rescbm5x0.res -o $@
endif

if OWCC
respet.res: respetcat.rc
	wrc -fo=respet.res -r -bt=nt \
	-i=$(top_builddir)/src \
	-i=$(top_srcdir)/src/arch/win32 \
	-i=$(top_srcdir)/src \
	respetcat.rc
else
respet.o: respetcat.rc
	$(WRC) -i respetcat.rc -o respet.res \
	--include-dir $(top_builddir)/src \
	--include-dir $(top_srcdir)/src/arch/win32 \
	--include-dir $(top_srcdir)/src
	$(WINDRES) -i respet.res -o $@
endif

if OWCC
resplus4.res: resplus4cat.rc
	wrc -fo=resplus4.res -r -bt=nt \
	-i=$(top_builddir)/src \
	-i=$(top_srcdir)/src/arch/win32 \
	-i=$(top_srcdir)/src \
	resplus4cat.rc
else
resplus4.o: resplus4cat.rc
	$(WRC) -i resplus4cat.rc -o resplus4.res \
	--include-dir $(top_builddir)/src \
	--include-dir $(top_srcdir)/src/arch/win32 \
	--include-dir $(top_srcdir)/src
	$(WINDRES) -i resplus4.res -o $@
endif

if OWCC
resvic20.res: resvic20cat.rc
	wrc -fo=resvic20.res -r -bt=nt \
	-i=$(top_builddir)/src \
	-i=$(top_srcdir)/src/arch/win32 \
	-i=$(top_srcdir)/src \
	resvic20cat.rc
else
resvic20.o: resvic20cat.rc
	$(WRC) -i resvic20cat.rc -o resvic20.res \
	--include-dir $(top_builddir)/src \
	--include-dir $(top_srcdir)/src/arch/win32 \
	--include-dir $(top_srcdir)/src
	$(WINDRES) -i resvic20.res -o $@
endif

if OWCC
resvsid.res: resvsidcat.rc
	wrc -fo=resvsid.res -r -bt=nt \
	-i=$(top_builddir)/src \
	-i=$(top_srcdir)/src/arch/win32 \
	-i=$(top_srcdir)/src \
	resvsidcat.rc
else
resvsid.o: resvsidcat.rc
	$(WRC) -i resvsidcat.rc -o resvsid.res \
	--include-dir $(top_builddir)/src \
	--include-dir $(top_srcdir)/src/arch/win32  \
	--include-dir $(top_srcdir)/src
	$(WINDRES) -i resvsid.res -o $@
endif

.PHONY: utils
utils:
	(cd utils; $(MAKE))
