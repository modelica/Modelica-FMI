#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libxml/parser.h>
#include <libxml/xmlschemas.h>


typedef struct {
    size_t len;
    char** messages;
} Messages;

static void log_error(void* ctx, const char* msg, ...) {
    
    assert(ctx);
    assert(msg);

    Messages* messages = (Messages*)ctx;

    char** temp = (char**)realloc(messages->messages, (messages->len + 1) * sizeof(char*));

    if (!temp) return;

    messages->messages = temp;
    messages->messages[messages->len] = NULL;

    va_list args;

    va_start(args, msg);
    const size_t len = vsnprintf(NULL, 0, msg, args);
    va_end(args);

    char* message = (char*)malloc(len + 1);

    assert(message);

    va_start(args, msg);
    vsnprintf(message, len + 1, msg, args);
    va_end(args);

    messages->messages[messages->len++] = message;
}

void free_messages(int len, char** messages) {

    assert(messages);
    
    for (size_t i = 0; i < len; i++) {
        free(messages[i]);
    }
    
    free(messages);
}

int validate_model_description(const char* model_description_path, int fmi_major_version, char*** messages) {
    return 0;
}

int validate_xml_document(
    const char* document_buffer, 
    int document_buffer_size, 
    const char* schema_buffer, 
    int schema_buffer_size, 
    char*** messages,
    xmlExternalEntityLoader external_entity_loader
) {
    Messages msg = { .len = 0, .messages = NULL };

    if (external_entity_loader) {
        xmlSetExternalEntityLoader(external_entity_loader);
    }

    xmlDocPtr doc = xmlParseMemory(document_buffer, document_buffer_size);

    if (!doc) {
        log_error(&msg, "Failed to parse document.");
        goto TERMINATE;
    }

    xmlNodePtr root = xmlDocGetRootElement(doc);

    if (root == NULL) {
        log_error(&msg, "Empty document.");
        goto TERMINATE;
    }

    xmlSchemaParserCtxtPtr pctxt = xmlSchemaNewMemParserCtxt(schema_buffer, schema_buffer_size);

    if (pctxt == NULL) {
        log_error(&msg, "Empty schema.");
        goto TERMINATE;
    }

    xmlSchemaPtr schema = xmlSchemaParse(pctxt);

    if (schema == NULL) {
        log_error(&msg, "Failed to parse XSD schema.");
        goto TERMINATE;
    }

    xmlSchemaValidCtxtPtr vctxt = xmlSchemaNewValidCtxt(schema);

    if (!vctxt) {
        log_error(&msg, "Failed to create validation context.");
        goto TERMINATE;
    }

    xmlSchemaSetValidErrors(vctxt, (xmlSchemaValidityErrorFunc)log_error, NULL, &msg);

    if (xmlSchemaValidateDoc(vctxt, doc)) {
        goto TERMINATE;
    }

TERMINATE:

    *messages = msg.messages;

    return msg.len;
}