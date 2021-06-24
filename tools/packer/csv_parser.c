#include<errno.h>

typedef struct {
    i64 *Data; // malloc()'ed
    int Width;
    int Height;
} map;

map parse_csv(char *Buffer, i64 BufferSize) {
    map Map = {0};
    i64 ScreenLen = 32*16;
    i64 AllocLength = ScreenLen;
    i64 UsedLength = 0;
    Map.Data = malloc(AllocLength*sizeof(*Map.Data));
    i64 SignMod = 1;
    pfori(BufferSize) {
        switch(Buffer[i]) {
            case '-': {
                SignMod = -1;
            } break;
            case 0: {
            } break;
            case ',': {
                SignMod = 1;
            } break;
            case '0': case '1': case '2': case '3': case '4': 
            case '5': case '6': case '7': case '8': case '9': {
                char *EndPtr;
                i64 Val = strtoll(Buffer+i, &EndPtr, 10);
                if(Val == LLONG_MIN || Val == LLONG_MAX || errno == ERANGE) {
                    printf("Failed to parse csv number at offset %lli\n", i);
                    exit(1);
                }
                if(UsedLength >= AllocLength) {
                    AllocLength += ScreenLen;
                    Map.Data = realloc(Map.Data, AllocLength*sizeof(*Map.Data));
                }
                if(Map.Height==0) Map.Width++;
                Map.Data[UsedLength++] = (u16)(Val*SignMod);
                i64 NewI = ((ptr)EndPtr)-((ptr)Buffer)-1;
                i = NewI;
            } break;
            case '\n': {
                Map.Height++;
            } break;
            default: {
                // just skip
            } break;
        }
    }
    if(UsedLength != Map.Width*Map.Height) {
        printf("Uneven row length is not allowed\n");
        exit(1);
    }
    Map.Data = realloc(Map.Data, UsedLength*sizeof(*Map.Data));
    return Map;
}