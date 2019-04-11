#include <stdio.h>
#include <string.h>

int main(int argc, char **argv)
{
    char buffer[1024];
    int bufIdx = 0;

    // init
    memset(buffer, 0x00, sizeof(buffer));

    // declear
    for (int i = 1; i < argc; ++i) {
        fprintf(stdout, "%d. param : '%s'\n", i, argv[i]);
        sprintf(buffer + bufIdx, "$%s = null;\n", argv[i]);
        bufIdx += strlen(buffer + bufIdx);
    }

    sprintf(buffer + bufIdx, "\n");
    bufIdx += strlen("\n");

    // isset
    for (int j = 1; j < argc; ++j) {
        sprintf(buffer + bufIdx, "if (isset($_POST[\"%s\"])) {\n\t$%s = $_POST[\"%s\"];\n}\n", argv[j], argv[j], argv[j]);
        bufIdx += strlen(buffer + bufIdx);
    }

    fprintf(stdout, "\n%s\n", buffer);
    return 0;
}