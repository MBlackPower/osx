/*
 * KLConfiguration.h
 *
 * $Header$
 *
 * Copyright 2003 Massachusetts Institute of Technology.
 * All Rights Reserved.
 *
 * Export of this software from the United States of America may
 * require a specific license from the United States Government.
 * It is the responsibility of any person or organization contemplating
 * export to obtain such a license before exporting.
 *
 * WITHIN THAT CONSTRAINT, permission to use, copy, modify, and
 * distribute this software and its documentation for any purpose and
 * without fee is hereby granted, provided that the above copyright
 * notice appear in all copies and that both that copyright notice and
 * this permission notice appear in supporting documentation, and that
 * the name of M.I.T. not be used in advertising or publicity pertaining
 * to distribution of the software without specific, written prior
 * permission.  Furthermore if you modify this software you must label
 * your software as modified software and not distribute it in such a
 * fashion that it might be confused with the original M.I.T. software.
 * M.I.T. makes no representations about the suitability of
 * this software for any purpose.  It is provided "as is" without express
 * or implied warranty.
 */

void __KLCallIdleCallback (void);

KLStatus __KLSetApplicationPrompter (KLPrompterProcPtr inPrompter);
KLStatus __KLGetApplicationPrompter (KLPrompterProcPtr *outPrompter);
KLBoolean __KLApplicationProvidedPrompter (void);
krb5_error_code __KLCallApplicationPrompter (krb5_context   context,
                                             void          *data,
                                             const char    *name,
                                             const char    *banner,
                                             int            num_prompts,
                                             krb5_prompt    prompts[]);

KLStatus __KLSetHomeDirectoryAccess (KLBoolean inAllowHomeDirectoryAccess);
KLStatus __KLGetHomeDirectoryAccess (KLBoolean *outAllowHomeDirectoryAccess);
KLBoolean __KLAllowHomeDirectoryAccess (void);

KLStatus __KLBeginPlugin (void);
KLStatus __KLEndPlugin (void);

KLStatus __KLSetAutomaticPrompting (KLBoolean inAllowAutomaticPrompting);
KLStatus __KLGetAutomaticPrompting (KLBoolean *outAllowAutomaticPrompting);
KLBoolean __KLAllowAutomaticPrompting (void);
