/****************************************************************************

giftool.c - GIF transformation tool.

****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <stdbool.h>

#include "getopt.h"
// #include "getarg.h"
#include "gif_lib.h"

#define PROGRAM_NAME	"giftool"

#define MAX_OPERATIONS	256
#define MAX_IMAGES	2048

enum boolmode { numeric, onoff, tf, yesno };

char *putbool(bool flag, enum boolmode mode)
{
	if (flag)
		switch (mode) {
		case numeric: return "1"; break;
		case onoff:   return "on"; break;
		case tf:      return "true"; break;
		case yesno:   return "yes"; break;
		}
	else
		switch (mode) {
		case numeric: return "0"; break;
		case onoff:   return "off"; break;
		case tf:      return "false"; break;
		case yesno:   return "no"; break;
		}

	return "FAIL";	/* should never happen */
}

bool getbool(char *from)
{
	struct valmap { char *name; bool val; }
	boolnames[] = {
		{ "yes", true },
		{ "on",  true },
		{ "1",   true },
		{ "t",   true },
		{ "no",  false },
		{ "off", false },
		{ "0",   false },
		{ "f",   false },
		{ NULL,  false },
	}, *sp;

	for (sp = boolnames; sp->name; sp++)
		if (strcmp(sp->name, from) == 0)
			return sp->val;

	(void)fprintf(stderr,
		"giftool: %s is not a valid boolean argument.\n",
		sp->name);
	exit(EXIT_FAILURE);
}

struct operation {
	enum {
		aspect,
		delaytime,
		background,
		info,
		interlace,
		position,
		screensize,
		transparent,
		userinput,
		disposal,
	} mode;
	union {
		GifByteType numerator;
		int delay;
		int color;
		int dispose;
		char *format;
		bool flag;
		struct {
			int x, y;
		} p;
	};
};

#define PrintGifError(x)

int gif_main(int argc, char **argv)
{
	extern char	*optarg;	/* set by getopt */
	extern int	optind;		/* set by getopt */
	struct operation operations[MAX_OPERATIONS];
	struct operation *top = operations;
	int selected[MAX_IMAGES], nselected = 0;
	bool have_selection = false;
	char *cp;
	int	i, status, ErrorCode;
	GifFileType *GifFileIn, *GifFileOut = (GifFileType *)NULL;
	struct operation *op;


	/* read in a GIF */
	if ((GifFileIn = DGifOpenFileHandle(0, &ErrorCode)) == NULL) {
		PrintGifError(ErrorCode);
		exit(EXIT_FAILURE);
	}
	if (DGifSlurp(GifFileIn) == GIF_ERROR) {
		PrintGifError(GifFileIn->Error);
		exit(EXIT_FAILURE);
	}
	if ((GifFileOut = EGifOpenFileHandle(1, &ErrorCode)) == NULL) {
		PrintGifError(ErrorCode);
		exit(EXIT_FAILURE);
	}

	/* if the selection is defaulted, compute it; otherwise bounds-check it */
	if (!have_selection)
		for (i = nselected = 0; i < GifFileIn->ImageCount; i++)
			selected[nselected++] = i;
	else
		for (i = 0; i < nselected; i++)
			if (selected[i] >= GifFileIn->ImageCount || selected[i] < 0)
			{
				(void)fprintf(stderr,
					"giftool: selection index out of bounds.\n");
				exit(EXIT_FAILURE);
			}

	/* perform the operations we've gathered */
	for (op = operations; op < top; op++)
		switch (op->mode)
		{
		case operation::background:
			GifFileIn->SBackGroundColor = op->color;
			break;

		case operation::delaytime:
			for (i = 0; i < nselected; i++)
			{
				GraphicsControlBlock gcb;

				DGifSavedExtensionToGCB(GifFileIn, selected[i], &gcb);
				gcb.DelayTime = op->delay;
				EGifGCBToSavedExtension(&gcb, GifFileIn, selected[i]);
			}
			break;

		case operation::info:
			for (i = 0; i < nselected; i++) {
				SavedImage *ip = &GifFileIn->SavedImages[selected[i]];
				GraphicsControlBlock gcb;
				for (cp = op->format; *cp; cp++) {
					if (*cp == '\\')
					{
						char c;
						switch (*++cp)
						{
						case 'b':
							(void)putchar('\b');
							break;
						case 'e':
							(void)putchar(0x1b);
							break;
						case 'f':
							(void)putchar('\f');
							break;
						case 'n':
							(void)putchar('\n');
							break;
						case 'r':
							(void)putchar('\r');
							break;
						case 't':
							(void)putchar('\t');
							break;
						case 'v':
							(void)putchar('\v');
							break;
						case 'x':
							switch (*++cp) {
							case '0':
								c = (char)0x00;
								break;
							case '1':
								c = (char)0x10;
								break;
							case '2':
								c = (char)0x20;
								break;
							case '3':
								c = (char)0x30;
								break;
							case '4':
								c = (char)0x40;
								break;
							case '5':
								c = (char)0x50;
								break;
							case '6':
								c = (char)0x60;
								break;
							case '7':
								c = (char)0x70;
								break;
							case '8':
								c = (char)0x80;
								break;
							case '9':
								c = (char)0x90;
								break;
							case 'A':
							case 'a':
								c = (char)0xa0;
								break;
							case 'B':
							case 'b':
								c = (char)0xb0;
								break;
							case 'C':
							case 'c':
								c = (char)0xc0;
								break;
							case 'D':
							case 'd':
								c = (char)0xd0;
								break;
							case 'E':
							case 'e':
								c = (char)0xe0;
								break;
							case 'F':
							case 'f':
								c = (char)0xf0;
								break;
							default:
								return -1;
							}
							switch (*++cp) {
							case '0':
								c += 0x00;
								break;
							case '1':
								c += 0x01;
								break;
							case '2':
								c += 0x02;
								break;
							case '3':
								c += 0x03;
								break;
							case '4':
								c += 0x04;
								break;
							case '5':
								c += 0x05;
								break;
							case '6':
								c += 0x06;
								break;
							case '7':
								c += 0x07;
								break;
							case '8':
								c += 0x08;
								break;
							case '9':
								c += 0x09;
								break;
							case 'A':
							case 'a':
								c += 0x0a;
								break;
							case 'B':
							case 'b':
								c += 0x0b;
								break;
							case 'C':
							case 'c':
								c += 0x0c;
								break;
							case 'D':
							case 'd':
								c += 0x0d;
								break;
							case 'E':
							case 'e':
								c += 0x0e;
								break;
							case 'F':
							case 'f':
								c += 0x0f;
								break;
							default:
								return -2;
							}
							putchar(c);
							break;
						default:
							putchar(*cp);
							break;
						}
					}
					else if (*cp == '%')
					{
						enum boolmode  boolfmt;
						SavedImage *sp = &GifFileIn->SavedImages[i];

						if (cp[1] == 't') {
							boolfmt = tf;
							++cp;
						}
						else if (cp[1] == 'o') {
							boolfmt = onoff;
							++cp;
						}
						else if (cp[1] == 'y') {
							boolfmt = yesno;
							++cp;
						}
						else if (cp[1] == '1') {
							boolfmt = numeric;
							++cp;
						}
						else
							boolfmt = numeric;

						switch (*++cp)
						{
						case '%':
							putchar('%');
							break;
						case 'a':
							(void)printf("%d", GifFileIn->AspectByte);
							break;
						case 'b':
							(void)printf("%d", GifFileIn->SBackGroundColor);
							break;
						case 'd':
							DGifSavedExtensionToGCB(GifFileIn,
								selected[i],
								&gcb);
							(void)printf("%d", gcb.DelayTime);
							break;
						case 'h':
							(void)printf("%d", ip->ImageDesc.Height);
							break;
						case 'n':
							(void)printf("%d", selected[i] + 1);
							break;
						case 'p':
							(void)printf("%d,%d",
								ip->ImageDesc.Left, ip->ImageDesc.Top);
							break;
						case 's':
							(void)printf("%d,%d",
								GifFileIn->SWidth,
								GifFileIn->SHeight);
							break;
						case 'w':
							(void)printf("%d", ip->ImageDesc.Width);
							break;
						case 't':
							DGifSavedExtensionToGCB(GifFileIn,
								selected[i],
								&gcb);
							(void)printf("%d", gcb.TransparentColor);
							break;
						case 'u':
							DGifSavedExtensionToGCB(GifFileIn,
								selected[i],
								&gcb);
							(void)printf("%s", putbool(gcb.UserInputFlag, boolfmt));
							break;
						case 'v':
							fputs(EGifGetGifVersion(GifFileIn), stdout);
							break;
						case 'x':
							DGifSavedExtensionToGCB(GifFileIn,
								selected[i],
								&gcb);
							(void)printf("%d", gcb.DisposalMode);
							break;
						case 'z':
							(void)printf("%s", putbool(sp->ImageDesc.ColorMap && sp->ImageDesc.ColorMap->SortFlag, boolfmt));
							break;
						default:
							(void)fprintf(stderr,
								"giftool: bad format %%%c\n", *cp);
						}
					}
					else
						(void)putchar(*cp);
				}
			}
			exit(EXIT_SUCCESS);
			break;

		case operation::interlace:
			for (i = 0; i < nselected; i++)
				GifFileIn->SavedImages[selected[i]].ImageDesc.Interlace = op->flag;
			break;

		case operation::position:
			for (i = 0; i < nselected; i++) {
				GifFileIn->SavedImages[selected[i]].ImageDesc.Left = op->p.x;
				GifFileIn->SavedImages[selected[i]].ImageDesc.Top = op->p.y;
			}
			break;

		case operation::screensize:
			GifFileIn->SWidth = op->p.x;
			GifFileIn->SHeight = op->p.y;
			break;

		case operation::transparent:
			for (i = 0; i < nselected; i++)
			{
				GraphicsControlBlock gcb;

				DGifSavedExtensionToGCB(GifFileIn, selected[i], &gcb);
				gcb.TransparentColor = op->color;
				EGifGCBToSavedExtension(&gcb, GifFileIn, selected[i]);
			}
			break;

		case operation::userinput:
			for (i = 0; i < nselected; i++)
			{
				GraphicsControlBlock gcb;

				DGifSavedExtensionToGCB(GifFileIn, selected[i], &gcb);
				gcb.UserInputFlag = op->flag;
				EGifGCBToSavedExtension(&gcb, GifFileIn, selected[i]);
			}
			break;

		case operation::disposal:
			for (i = 0; i < nselected; i++)
			{
				GraphicsControlBlock gcb;

				DGifSavedExtensionToGCB(GifFileIn, selected[i], &gcb);
				gcb.DisposalMode = op->dispose;
				EGifGCBToSavedExtension(&gcb, GifFileIn, selected[i]);
			}
			break;

		default:
			(void)fprintf(stderr, "giftool: unknown operation mode\n");
			exit(EXIT_FAILURE);
		}

	/* write out the results */
	GifFileOut->SWidth = GifFileIn->SWidth;
	GifFileOut->SHeight = GifFileIn->SHeight;
	GifFileOut->SColorResolution = GifFileIn->SColorResolution;
	GifFileOut->SBackGroundColor = GifFileIn->SBackGroundColor;
	GifFileOut->SColorMap = GifMakeMapObject(
		GifFileIn->SColorMap->ColorCount,
		GifFileIn->SColorMap->Colors);

	for (i = 0; i < GifFileIn->ImageCount; i++)
		(void) GifMakeSavedImage(GifFileOut, &GifFileIn->SavedImages[i]);

	if (EGifSpew(GifFileOut) == GIF_ERROR)
		PrintGifError(GifFileOut->Error);
	else if (DGifCloseFile(GifFileIn, &ErrorCode) == GIF_ERROR)
		PrintGifError(ErrorCode);

	return 0;
}

/* end */
