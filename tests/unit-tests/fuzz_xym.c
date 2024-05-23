#include "xym/format/printers.h"
#include "xym/format/format.h"
#include "xym/parse/xym_parse.h"
#include "buffer.h"
#include "apdu/global.h"

#include <stdlib.h>

transaction_context_t transactionContext;
fields_array_t  *fields = NULL;

char *fieldName = NULL;
char *fieldValue = NULL;


void init_globals() {
    static bool initialized = false;

    if (!initialized) {
        fieldName = malloc(MAX_FIELDNAME_LEN);
        fieldValue = malloc(MAX_FIELD_LEN);
        fields = malloc(sizeof(fields_array_t));

        initialized = true;
    }
}

int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
    init_globals();

    buffer_t buf = {Data, Size, 0};
    if (parse_txn_context(&buf, fields) != E_SUCCESS) {
        return 0;
    }

    for (int i = 0; i < fields->numFields; i++) {
        field_t *field = &fields->arr[i];
        resolve_fieldname(field, fieldName);
        memset(fieldValue, 0, MAX_FIELD_LEN);
        format_field(field, fieldValue);
        printf("%s: %s\n", fieldName, fieldValue);    }
    return 0;
}
