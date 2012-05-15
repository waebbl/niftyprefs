/*
 * libniftyprefs - lightweight modelless preferences management library
 * Copyright (C) 2006-2012 Daniel Hiepler <daniel@niftylight.de>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Alternatively, the contents of this file may be used under the
 * GNU Lesser General Public License Version 2.1 (the "LGPL"), in
 * which case the following provisions apply instead of the ones
 * mentioned above:
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/**
 * @file niftyprefs.h
 */


/**      
 * @mainpage preferences handling for arbitrary objects
 *
 * <h1>Introduction</h1>
 * The basic idea is to define and manage "classes" for arbitrary (void *)
 * pointers in a comfortable way. So you simply can:
 * - "snapshot" the state of an object for saving it in a preferences-definition 
 * - create an object from a previously created "snapshot"
 *
 *
 * <h1>Usage</h1>
 *
 * In every case call:
 *  - nft_prefs_init() before doing anything
 *  - nft_prefs_class_register() for every object-class, before using the class 
 *    with any of nft_prefs_*()
 *  - ...
 *  - nft_prefs_obj_unregister() when freeing resources of a registered object
 *  - ...
 *  - nft_prefs_class_unregister() when a class is not used anymore
 *  - nft_prefs_exit() when no nft_prefs_*() needs to be called anymore
 *
 *
 * Use case 1: Generate preferences from existing object
 *  - ...
 *  - [create object]
 *  - nft_prefs_obj_register(obj)
 *  - prefsObj = nft_prefs_from_obj(obj)
 *  - [use prefsObj]
 *  - nft_prefs_obj_free(prefsObj)
 *
 * Use case 2: Generate object from preferences
 *  - ...
 *  - create NftPrefsNode nft_prefs_node_from_file() / nft_prefs_node_from_buffer()
 *  - create object(s) from NftPrefsNode
 *
 * Use case 3: Dump preferences to file
 * Use case 4: Dump preferences to string
 * Use case 5: Parse preferences from file
 * Use case 6: Parse preferences from string
 *
 * @defgroup prefs niftyprefs
 * @brief abstract preference handling for arbitrary objects
 * @{
 */

#ifndef _NIFTYPREFS_H
#define _NIFTYPREFS_H


#include <niftylog.h>
#include <libxml/tree.h>



/** maximum length of a classname */
#define NFT_PREFS_MAX_CLASSNAME 64


/** a context holding a list of PrefsClasses and PrefsNodes - acquired by nft_prefs_init() */
typedef struct _NftPrefs NftPrefs;
/** an object descriptor that holds various properties about an object */
typedef struct _NftPrefsObj NftPrefsObj;
/** wrapper type for one xmlNode */
typedef xmlNode NftPrefsNode;


/** 
 * function that creates a config-node for a certain object 
 *
 * @param p current NftPrefs context
 * @param newNode newly created empty node that will be filled by the function
 * @param obj the object to process
 * @param userptr arbitrary pointer defined upon registering the object class
 * @result NFT_SUCCESS or NFT_FAILURE (processing will be aborted upon failure)
 */
typedef NftResult (NftPrefsFromObjFunc)(NftPrefs *p, NftPrefsNode *newNode, 
                                        void *obj, void *userptr);


/** 
 * function that allocates a new object from a config-node 
 *
 * @param p current NftPrefs context
 * @param newObj space to put the pointer to a newly allocated object
 * @param node the preference node describing the object that's about to be created
 * @param userptr arbitrary pointer defined upon registering the object class
 * @result NFT_SUCCESS or NFT_FAILURE (processing will be aborted upon failure)
 */
typedef NftResult (NftPrefsToObjFunc)(NftPrefs *p, void **newObj, 
                                      NftPrefsNode *node, void *userptr);




NftPrefs *      nft_prefs_init();
void            nft_prefs_exit(NftPrefs *prefs);
void            nft_prefs_free(void *p);


NftResult       nft_prefs_class_register(NftPrefs *p, const char *className, NftPrefsToObjFunc *toObj, NftPrefsFromObjFunc *fromObj);
void            nft_prefs_class_unregister(NftPrefs *p, const char *className);


NftResult       nft_prefs_obj_register(NftPrefs *p, const char *className, void *obj);
void            nft_prefs_obj_unregister(NftPrefs *p, const char *className, void *obj);

void *          nft_prefs_obj_from_file(NftPrefs *p, const char *filename, void *userptr);
void *          nft_prefs_obj_from_buffer(NftPrefs *p, char *buffer, size_t bufsize, void *userptr);
void *          nft_prefs_obj_from_node(NftPrefs *p, NftPrefsNode *n, void *userptr);

NftResult       nft_prefs_obj_to_file(NftPrefs *p, const char *className, void *obj, const char *filename, void *userptr);
char *          nft_prefs_obj_to_buffer(NftPrefs *p, const char *className, void *obj, void *userptr);
NftPrefsNode *  nft_prefs_obj_to_node(NftPrefs *p, const char *className, void *obj, void *userptr);


NftResult       nft_prefs_node_add_child(NftPrefsNode *parent, NftPrefsNode *cur);
NftPrefsNode *  nft_prefs_node_get_first_child(NftPrefsNode *n);
NftPrefsNode *  nft_prefs_node_get_next(NftPrefsNode *n);

NftResult       nft_prefs_node_prop_string_set(NftPrefsNode *n, const char *name, char *value);
char *          nft_prefs_node_prop_string_get(NftPrefsNode *n, const char *name);
NftResult       nft_prefs_node_prop_int_set(NftPrefsNode *n, const char *name, int val);
NftResult       nft_prefs_node_prop_int_get(NftPrefsNode *n, const char *name, int *val);


#endif /* _NIFTYPREFS_H */


/**
 * @}
 */
