#ifndef SCM_GPIB_H
#define SCM_GPIB_H 1

#if defined (SCM_MINOR_VERSION) && (SCM_MINOR_VERSION >= 5) && \
    defined (SCM_MAJOR_VERSION) && (SCM_MAJOR_VERSION >= 1)
#define SCM_VERSION_15X 1
#endif
#if defined (SCM_MINOR_VERSION) && (SCM_MINOR_VERSION >= 7) && \
    defined (SCM_MAJOR_VERSION) && (SCM_MAJOR_VERSION >= 1)
#define SCM_VERSION_17X 1
#endif

#ifndef SCM_VERSION_17X
#define scm_c_string2str(obj, str, lenp) gh_scm2newstr (obj, lenp)
#endif
#ifndef SCM_VERSION_17X
#define scm_c_symbol2str(obj, str, lenp) gh_symbol2newstr (obj, lenp)
#endif
#define guile_to_string(cell)                                         \
  (SCM_NULLP (cell) ? NULL :                                          \
  (SCM_STRINGP (cell) ? scm_c_string2str (cell, NULL, NULL) :         \
  (SCM_SYMBOLP (cell) ? scm_c_symbol2str (cell, NULL, NULL) : NULL)))
#ifndef SCM_EXACTP
#define SCM_EXACTP(obj) SCM_NFALSEP (scm_exact_p (obj))
#endif
#ifndef SCM_NUM2INT
#define SCM_NUM2INT(pos, obj) gh_scm2int (obj)
#endif
#ifndef SCM_STRING_CHARS
#define SCM_STRING_CHARS(obj) ((char *) SCM_VELTS (obj))
#endif
#ifndef SCM_VERSION_15X
#define scm_c_define_gsubr(name, req, opt, rst, fcn) \
    gh_new_procedure (name, fcn, req, opt, rst)
#endif
#ifndef SCM_VERSION_15X
#define scm_int2num(x) scm_long2num ((long) (x))
#endif
#ifndef SCM_VERSION_15X
#define scm_mem2string(str, len) gh_str2scm (str, len)
#endif
#ifndef SCM_VERSION_15X
#define scm_c_define(name, val) gh_define (name, val)
#endif

#endif /* SCM_GPIB_H */
