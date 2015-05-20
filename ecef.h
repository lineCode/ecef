#define CEF_NEW(TYPE) cef_new(sizeof(TYPE), NULL)
#define CEF_NEW_PTR(TYPE, PTR) cef_new(sizeof(TYPE), (void**)PTR)

//#define CEF_INCLUDE_INTERNAL_CEF_EXPORT_H_
//#define CEF_CALLBACK
//#define CEF_EXPORT __attribute__((weak))

#include "include/capi/cef_base_capi.h"
#include "include/capi/cef_browser_capi.h"
#include "include/capi/cef_client_capi.h"
#include "include/capi/cef_render_handler_capi.h"
#include "include/capi/cef_render_process_handler_capi.h"
#include "include/capi/cef_display_handler_capi.h"
#include <assert.h>

#ifndef TEST_APP
#include <Elementary.h>
#include <Evas.h>
#define WEIGHT evas_object_size_hint_weight_set
#define ALIGN evas_object_size_hint_align_set
#define EXPAND(X) WEIGHT((X), EVAS_HINT_EXPAND, EVAS_HINT_EXPAND)
#define FILL(X) ALIGN((X), EVAS_HINT_FILL, EVAS_HINT_FILL)
#endif
typedef struct Ref
{
   unsigned int count;
   void **ptr;
} Ref;

static inline void *
getref(cef_base_t *self)
{
   return (char*)self + self->size;
}

static inline void
cef_addref(void *self)
{
   Ref *ref = getref(self);
   ++ref->count;
   //fprintf(stderr, "REF(%p) %d\n", self, ref->count);
}

static inline int
cef_delref(void *self)
{
   Ref *ref = getref(self);
   //fprintf(stderr, "UNREF(%p) %d\n", self, ref->count - 1);
   if (!(--ref->count))
     {
        if (ref->ptr)
          *ref->ptr = NULL;
        free(self);
        return 1;
     }
   return 0;
}

static inline int
cef_has_one_ref(void *self)
{
   Ref *ref = getref(self);
   return ref->count == 1;
}

static inline void
cef_init(cef_base_t *base)
{
   base->add_ref = (void*)cef_addref;
   base->release = (void*)cef_delref;
   base->has_one_ref = (void*)cef_has_one_ref;
   base->add_ref(base);
}

static inline void *
cef_new(size_t size, void **ptr)
{
   cef_base_t *base = calloc(1, size + sizeof(Ref));
   Ref *ref;

   base->size = size;
   cef_init(base);
   ref = getref(base);
   ref->ptr = ptr;
   if (ptr)
     *ptr = base;

   return base;
}
#ifndef TEST_APP
typedef struct Browser
{
   cef_browser_t *browser;
   Evas_Object *img;
   Eina_Stringshare *title;
   Eina_Stringshare *url;
   Elm_Object_Item *it;
   GLuint program;
   GLuint vao;
   GLuint vbo;
   GLuint texture_id;
   void *buffer;
#ifdef HAVE_SERVO
   int pw, ph;
#endif
   int w, h;
} Browser;

typedef struct ECef_Client
{
   cef_client_t client;
   Evas_Object *win;
   Evas_Object *layout;
   Evas_Object *pagelist;
   Evas_Object *urlbar;
   Browser *current_page;
   Eina_Hash *browsers;
   cef_render_handler_t *render_handler;
   cef_display_handler_t *display_handler;
   cef_window_info_t *window_info;
   cef_browser_settings_t *browser_settings;
} ECef_Client;


cef_render_handler_t *client_render_handler_get(cef_client_t *client);
cef_display_handler_t *client_display_handler_get(cef_client_t *client);

static inline cef_browser_host_t *
browser_get_host(cef_browser_t *browser)
{
   return browser->get_host(browser);
}
static inline ECef_Client *
browser_get_client(cef_browser_t *browser)
{
   return (void*)browser_get_host(browser)->get_client(browser_get_host(browser));
}

static inline int
modifiers_get(Evas_Modifier *m) {
  int modifiers = 0;
  if (evas_key_modifier_is_set(m, "Shift_L") || evas_key_modifier_is_set(m, "Shift_R"))
    modifiers |= EVENTFLAG_SHIFT_DOWN;
  if (evas_key_modifier_is_set(m, "Caps_Lock"))
    modifiers |= EVENTFLAG_CAPS_LOCK_ON;
  if (evas_key_modifier_is_set(m, "Control_L") || evas_key_modifier_is_set(m, "Control_R"))
    modifiers |= EVENTFLAG_CONTROL_DOWN;
  if (evas_key_modifier_is_set(m, "Alt_L") || evas_key_modifier_is_set(m, "Alt_R"))
    modifiers |= EVENTFLAG_ALT_DOWN;
  return modifiers;
}

void on_after_browser_created(cef_life_span_handler_t *self EINA_UNUSED, cef_browser_t *browser);
void browser_new(ECef_Client *ec, const char *url);
Browser *browser_get(ECef_Client *ec, cef_browser_t *browser);
void browser_set(ECef_Client *ec, Browser *b);
void browser_urlbar_show(ECef_Client *ec, Eina_Bool changed);
void browser_urlbar_hide(ECef_Client *ec);
void browser_pagelist_show(ECef_Client *ec);
void browser_pagelist_hide(ECef_Client *ec);

void render_image_new(ECef_Client *ec, Browser *b, cef_browser_host_t *host, int w, int h);

extern Eina_Bool servo;
extern Eina_Bool gl_avail;

# define E_LIST_HANDLER_APPEND(list, type, callback, data) \
  do \
    { \
       Ecore_Event_Handler *_eh; \
       _eh = ecore_event_handler_add(type, (Ecore_Event_Handler_Cb)callback, data); \
       assert(_eh); \
       list = eina_list_append(list, _eh); \
    } \
  while (0)
#endif
