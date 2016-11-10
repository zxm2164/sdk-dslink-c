#define LOG_TAG "serialization"

#include <dslink/log.h>
#include "serialization.h"

static
void save_node(uv_timer_t* timer) {
    DSNode *node = timer->data;
    timer->data = NULL;
    json_t *json = dslink_node_serialize(node);
    json_dump_file(json, "saved_node.json", 0);
    json_decref(json);
}
static
uv_timer_t save_timer = {0};
static
void on_node_changed(struct DSLink *link, DSNode *node) {
    if (save_timer.data){
        return;
    }
    save_timer.data = node;
    uv_timer_init(&link->loop, &save_timer);
    uv_timer_start(&save_timer, save_node, 100, 0);
}

static
void load_node(DSNode *node) {
    json_error_t err;
    json_t *json = json_load_file("saved_node.json", 0 , &err);
    if (json) {
        dslink_node_deserialize(node, json);
        json_decref(json);
    }
}

void responder_init_serialization(DSLink *link, DSNode *root) {
    DSNode *node = dslink_node_create(root, "saved", "node");
    node->on_data_changed = on_node_changed;
    load_node(node);
    dslink_node_set_meta_new(link, node, "$writable", json_string_nocheck("write"));
    dslink_node_set_meta_new(link, node, "$type", json_string_nocheck("string"));
    if (dslink_node_add_child(link, node) != 0) {
        log_warn("Failed to add the serialization node to the root\n");
        dslink_node_tree_free(link, node);
    }
}
