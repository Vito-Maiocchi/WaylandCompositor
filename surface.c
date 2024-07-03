#include <wayland-util.h>
#include <wayland-server-core.h>
#include <wlr/types/wlr_xdg_shell.h>
#include <wlr/types/wlr_scene.h>

#include <assert.h>
#include <stdlib.h>

#include "layout.h"
#include "wayland.h"

struct XdgSurface {
	struct wlr_xdg_surface* xdg_surface;
	struct wl_list popups; //List of popup xdg_surfaces

	struct wlr_scene_tree* surface_node; //Surface node: holds the surface itself without decorations
	struct SurfaceDecorations decorations;

	struct wl_listener map;
	struct wl_listener unmap;
	struct wl_listener destroy;
};

struct LayerShellSurface {
};

struct SurfaceDecorations {
	struct wlr_scene_rect* border[4]; // Borders of the window in the following order: top, bottom, left, right
};

//Surface invariant refers to this: wlr_surface->data = Surface
//The "Surface invariant" property must allways be upheld.
//This is necessary to allways get the Surface struct from its children.
struct Surface {
    struct wl_list link;

	struct wlr_scene_tree* root_node; //Root node of the window. It is the parent of the surface and decorations
	
    enum {
        SURFACE_TYPE_XDG,
        SURFACE_TYPE_LAYER_SHELL
    } type;

    union {
        struct XdgSurface* xdg;
        struct LayerShellSurface* layer_shell;
    } details;

    struct LayoutNode layoutNode;

	enum {
		SURFACE_STATE_MAPPED,	//The surface is mapped and managed by LAYOUT.
		SURFACE_STATE_UNMAPPED, //The surface is not mapped and not managed by LAYOUT.
		SURFACE_STATE_HIDDEN	//The surface is not mapped but managed by the LAYOUT.
	} state;
    
};

typedef struct Surface* Surface;
typedef struct XdgSurface* XdgSurface;

static Surface focused_surface = NULL;

static void surface_set_focused(Surface surface) {

}

static void surface_decoration_destroy(Surface surface) {

}

//Converts listener into Surface that contains it
#define surface_xdg_of(listener, member) 	\
	((XdgSurface)((char *)(listener) - 		\
	offsetof(struct XdgSurface, member))) 	\
	->xdg_surface->surface->data

static void xdg_surface_map_notify(struct wl_listener* listener, void* data) {  //DI GANZ MAPPING GSCHICHT LAUFT GLAUB NÖD SO
	Surface surface = surface_xdg_of(listener, map);
	assert(surface); //verify Surface invariant

	layout_add_client(&surface->layoutNode);
	surface_set_focused(surface);
}

static void xdg_surface_unmap_notify(struct wl_listener* listener, void* data) {
	Surface surface = surface_xdg_of(listener, unmap);
	assert(surface); //verify Surface invariant

	surface_decoration_destroy(surface);

	struct LayoutNode* node = layout_remove_client(&surface->layoutNode);
	if(focused_surface = surface) {
		focused_surface = NULL;
		Surface s = wl_container_of(node, s, layoutNode);
		surface_set_focused(s);
	}
}

static void xdg_surface_destroy_notify(struct wl_listener* listener, void* data) {
	//wlr_surface_
}

static void xdg_surface_new_notify(struct wl_listener* listener, void* data) {
	struct wlr_xdg_surface *xdg_surface = data;

	//HANDLE POPUP CASE
	if (xdg_surface->role == WLR_XDG_SURFACE_ROLE_POPUP) { //FIXME: ka ob das so funktioniert
		struct wlr_surface* parent = xdg_surface->popup->parent;
		assert(parent);
		assert(parent->data); //if this fails "wlr_surface->data = Surface" is violeted
		Surface surface = parent->data;
		assert(surface->type == SURFACE_TYPE_XDG);
		xdg_surface->surface->data = surface;
		struct wlr_scene_tree* surface_node = wlr_scene_xdg_surface_create(surface->details.xdg->surface_node, xdg_surface);
		wl_list_insert(&surface->details.xdg->popups, &xdg_surface->link);
		return;
	}

	//HANDLE TOPLEVEL CASE
	assert(xdg_surface->role == WLR_XDG_SURFACE_ROLE_TOPLEVEL);

	Surface surface = calloc(1, sizeof(struct Surface));
	surface->type = SURFACE_TYPE_XDG;
	surface->root_node = wlr_scene_tree_create(&server.scene->tree);
	
	surface->details.xdg = calloc(1, sizeof(struct XdgSurface));
	struct XdgSurface* xdgSurface = surface->details.xdg;
	xdgSurface->xdg_surface = xdg_surface;
	xdgSurface->surface_node = wlr_scene_xdg_surface_create(surface->root_node, xdg_surface);
	xdg_surface->surface->data = surface;

	//CONFIGURE LISTENERS
	xdgSurface->map.notify		= xdg_surface_map_notify;
	xdgSurface->unmap.notify	= xdg_surface_unmap_notify;
	xdgSurface->destroy.notify 	= xdg_surface_destroy_notify;
	wl_signal_add(&xdg_surface->surface->events.map, 		&xdgSurface->map);
	wl_signal_add(&xdg_surface->surface->events.unmap, 		&xdgSurface->unmap);
	wl_signal_add(&xdg_surface->surface->events.destroy, 	&xdgSurface->destroy);
}

void surface_setup(struct wl_display* display) {

    struct wlr_xdg_shell* xdg_shell;
    struct wl_listener xdg_new_surface_listener;

    //XDG SHELL
    xdg_shell = wlr_xdg_shell_create(display, 3);
	xdg_new_surface_listener.notify = xdg_surface_new_notify;
	wl_signal_add(&xdg_shell->events.new_surface, &xdg_new_surface_listener);

}