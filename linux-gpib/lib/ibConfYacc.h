#ifndef BISON_IBCONFYACC_H
# define BISON_IBCONFYACC_H

#ifndef YYSTYPE
typedef union
{
int  ival;
char *sval;
char bval;
char cval;
} yystype;
# define YYSTYPE yystype
# define YYSTYPE_IS_TRIVIAL 1
#endif

#ifndef YYLTYPE
typedef struct yyltype
{
  int first_line;
  int first_column;

  int last_line;
  int last_column;
} yyltype;

# define YYLTYPE yyltype
# define YYLTYPE_IS_TRIVIAL 1
#endif

# define	T_INTERFACE	257
# define	T_DEVICE	258
# define	T_NAME	259
# define	T_MINOR	260
# define	T_BASE	261
# define	T_IRQ	262
# define	T_DMA	263
# define	T_PAD	264
# define	T_SAD	265
# define	T_TIMO	266
# define	T_EOSBYTE	267
# define	T_BOARD_TYPE	268
# define	T_PCI_BUS	269
# define	T_PCI_SLOT	270
# define	T_REOS	271
# define	T_BIN	272
# define	T_INIT_S	273
# define	T_DCL	274
# define	T_XEOS	275
# define	T_EOT	276
# define	T_MASTER	277
# define	T_LLO	278
# define	T_EXCL	279
# define	T_INIT_F	280
# define	T_AUTOPOLL	281
# define	T_NUMBER	282
# define	T_STRING	283
# define	T_BOOL	284
# define	T_TIVAL	285


#endif /* not BISON_IBCONFYACC_H */
