/*
 * libniftyprefs - lightweight modelless preferences management library
 * Copyright (C) 2006-2014 Daniel Hiepler <daniel@niftylight.de>
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


#include <stdlib.h>
#include <niftylog.h>
#include <niftyprefs.h>


/* amount of example persons */
#define PEOPLECOUNT 2


/* one "object" */
struct Person
{
        char name[256];
        char email[256];
        int age;
        bool alive;
} persons[PEOPLECOUNT];

/* our toplevel object */
struct People
{
        struct Person *people[PEOPLECOUNT];
        size_t people_count;
} people;


/* printable name of "objects" */
#define PEOPLE_NAME "people"
#define PERSON_NAME "person"


/* file to read from */
#define FILE_NAME   "test.xml"


/******************************************************************************/

/** function to generate a People "object" from a preferences description */
static NftResult _people_from_prefs(NftPrefs * p, void **newObj,
                                    NftPrefsNode * node, void *userptr)
{
        people.people_count = sizeof(persons) / sizeof(struct Person);

        /* call toObj() of child objects */
        NftPrefsNode *child;
        size_t i = 0;
        for(child = nft_prefs_node_get_first_child(node);
            child; child = nft_prefs_node_get_next(child))
        {
                if(i >= sizeof(persons) / sizeof(struct Person))
                {
                        NFT_LOG(L_ERROR,
                                "more persons in prefs file than expected");
                        return NFT_FAILURE;
                }

                /* call toObj function of child node (should be a <person>
                 * node) */
                if(!
                   (people.people[i++] =
                    nft_prefs_obj_from_node(p, child, userptr)))
                {
                        NFT_LOG(L_ERROR,
                                "Failed to create object from preference node");
                        return NFT_FAILURE;
                }
        }

        /* save pointer to new object */
        *newObj = &people;

        return NFT_SUCCESS;
}


/** function to generate a Person "object" from a preferences description */
static NftResult _person_from_prefs(NftPrefs * p, void **newObj,
                                    NftPrefsNode * node, void *userptr)
{
        static size_t i;

        /* only fill reserved space, not more */
        if(i >= sizeof(persons) / sizeof(struct Person))
                return NFT_FAILURE;

        char *name;
        if(!(name = nft_prefs_node_prop_string_get(node, "name")))
        {
                NFT_LOG(L_ERROR,
                        "failed to get property \"name\" from prefs-node");
                return NFT_FAILURE;
        }

        char *email;
        if(!(email = nft_prefs_node_prop_string_get(node, "email")))
        {
                NFT_LOG(L_ERROR,
                        "failed to get property \"email\" from prefs-node");
                return NFT_FAILURE;
        }

        int age;
        if(!nft_prefs_node_prop_int_get(node, "age", &age))
        {
                NFT_LOG(L_ERROR,
                        "failed to get property \"age\" from prefs-node");
                return NFT_FAILURE;
        }

        bool alive;
        if(!nft_prefs_node_prop_boolean_get(node, "alive", &alive))
        {
                NFT_LOG(L_ERROR,
                        "failed to get property \"alive\" from prefs-node");
                return NFT_FAILURE;
        }

        strncpy(persons[i].name, name, sizeof(persons[i].name));
        strncpy(persons[i].email, email, sizeof(persons[i].email));
        persons[i].age = age;
        persons[i].alive = alive;

        /* free strings */
        nft_prefs_free(name);
        nft_prefs_free(email);

        /* save pointer to new object */
        *newObj = &persons[i++];


        return NFT_SUCCESS;
}


/** create object from preferences definition */
int main(int argc, char *argv[])
{
        /* do preliminary version checks */
        if(!NFT_PREFS_CHECK_VERSION)
                return EXIT_FAILURE;

        /* fail per default */
        int result = EXIT_FAILURE;


        /* initialize libniftyprefs */
        NftPrefs *prefs;
        if(!(prefs = nft_prefs_init(0)))
        {
                NFT_LOG(L_ERROR, "initialize prefs");
                goto _deinit;
        }


        /* register "people" class to niftyprefs */
        if(!
           (nft_prefs_class_register
            (prefs, PEOPLE_NAME, &_people_from_prefs, NULL)))
        {
                NFT_LOG(L_ERROR, "failed to register class");
                goto _deinit;
        }

        /* register "person" class to niftyprefs */
        if(!
           (nft_prefs_class_register
            (prefs, PERSON_NAME, &_person_from_prefs, NULL)))
        {
                NFT_LOG(L_ERROR, "failed to register class");
                goto _deinit;
        }


        /* parse file to prefs node */
        NftPrefsNode *node;
        if(!(node = nft_prefs_node_from_file(prefs, "test-prefs.xml")))
        {
                NFT_LOG(L_ERROR,
                        "failed to parse prefs file \"test-prefs.xml\"");
                goto _deinit;
        }

        /* create object from node */
        struct People *people;
        if(!(people = nft_prefs_obj_from_node(prefs, node, NULL)))
        {
                NFT_LOG(L_ERROR, "failed to create object from prefs node");
                goto _deinit;
        }

        /* free node */
        nft_prefs_node_free(node);

        /* process all persons */
        size_t n;
        for(n = 0; n < people->people_count; n++)
        {
                /* print info */
                printf("\tperson(name=\"%s\",email=\"%s\", age=\"%d\", vitality=\"%s\")\n", people->people[n]->name, people->people[n]->email, people->people[n]->age, people->people[n]->alive ? "alive" : "dead");
        }

        /* we should get what we put in */
        if((strcmp(people->people[0]->name, "Bob") != 0) ||
           (strcmp(people->people[0]->email, "bob@example.com") != 0) ||
           (people->people[0]->age != 30) ||
           (people->people[0]->alive != true) ||
           (strcmp(people->people[1]->name, "Alice") != 0) ||
           (strcmp(people->people[1]->email, "alice@example.com") != 0) ||
           (people->people[1]->age != 30) ||
           (people->people[1]->alive != false))
        {
                NFT_LOG(L_ERROR,
                        "Input from obj-to-prefs.c doesn't match output!");
                goto _deinit;
        }

                /** @todo check xml against DTD */

        /* all went fine */
        result = EXIT_SUCCESS;

_deinit:
        nft_prefs_deinit(prefs);

        return result;
}
