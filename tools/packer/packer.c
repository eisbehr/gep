#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
typedef float r32;
typedef double r64;

typedef float f32;
typedef double f64;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int32_t b32;

typedef uintptr_t uptr;
typedef intptr_t ptr;

#define Assert(Expression) if(!(Expression)) {int *x = 0; *x;}

#define ArrayLength(Arr) (sizeof(Arr)/sizeof(Arr[0]))
#define ArraySize(Arr) (sizeof(Arr))

#define pfor(counter, max) for(i64 (counter)=0; (counter)<(max); (counter)++)
#define pfori(max) for(i64 i=0; i<(max); i++)
#define pforj(max) for(i64 j=0; j<(max); j++)
#define pfork(max) for(i64 k=0; k<(max); k++)
#define pforx(max) for(i64 x=0; x<(max); x++)
#define pfory(max) for(i64 y=0; y<(max); y++)
#define pforz(max) for(i64 z=0; z<(max); z++)

#define pfor(counter, max) for(i64 (counter)=0; (counter)<(max); (counter)++)
#define pfori32(max) for(i32 i=0; i<(max); i++)
#define pforj32(max) for(i32 j=0; j<(max); j++)
#define pfork32(max) for(i32 k=0; k<(max); k++)
#define pforx32(max) for(i32 x=0; x<(max); x++)
#define pfory32(max) for(i32 y=0; y<(max); y++)
#define pforz32(max) for(i32 z=0; z<(max); z++)

#define pforu(counter, max) for(u64 (counter)=0; (counter)<(max); (counter)++)
#define pforiu(max) for(u64 i=0; i<(max); i++)
#define pforju(max) for(u64 j=0; j<(max); j++)
#define pforku(max) for(u64 k=0; k<(max); k++)
#define pforxu(max) for(u64 x=0; x<(max); x++)
#define pforyu(max) for(u64 y=0; y<(max); y++)
#define pforzu(max) for(u64 z=0; z<(max); z++)

#define Kilobytes(size) (ptr)size*1024
#define Megabytes(size) Kilobytes((ptr)size*1024)
#define Gigabytes(size) Megabytes((ptr)size*1024)

#include "csv_parser.c"

typedef union {
    struct {
        u8 BackgroundColor;
        u16 Filler;
        u8 PaletteInfo;
    } cga_monochrome;
    u8 Color16[48]; // rgb...
    u8 Color256[48];
} pcx_color_table;

typedef struct {
    u8 Magic;
    u8 Version;
    u8 Compress;
    u8 BitsPerPixel;
    u16 Xmin;
    u16 Ymin;
    u16 Xmax;
    u16 Ymax;
    u16 Xdpi;
    u16 Ydpi;
    pcx_color_table ColorTable;
    u8 Reserved;
    u8 NumColorplanes;
    u16 BytesPerScanline;
    u16 ColorTableType;
    u8 Filler[58];
} pcx_hdr;

typedef enum {
    mode_none,
    mode_pcx,
    mode_csv,
} packer_mode;

int main(int argc, char **argv) {
    
    if(argc < 5) {
        usage:
        printf("Usage: [pcx|csv] [input file] [variable name] [output.h file]\npcx mode is meant for tile-sprite-memory maps, csv mode is meant \nfor loading maps from the 'Tiled' tile map editor. Choose one of \nthose options as the first argument, then supply all of the other \narguments\n");
        return 1;
    }
    
    packer_mode Mode = 0;
    if(strcmp(argv[1], "pcx") == 0) Mode = mode_pcx;
    else if(strcmp(argv[1], "csv") == 0) Mode = mode_csv;
    else {
        printf("Unknown filetype selected '%s'\n\n", argv[1]);
        goto usage;
    }
    
    char *InputFileStr = argv[2];
    char *VarNameStr = argv[3];
    char *OutputFileStr = argv[4];
    
    FILE *File = fopen(InputFileStr, "rb");
    if(!File) {
        printf("Failed to open file \n");
        return 1;
    }
    
    int SeekRes = fseek(File, 0, SEEK_END);
    int FileSize = ftell(File);
    if(FileSize < 0) {
        printf("ftell() failed\n");
        return 1;
    }
    SeekRes &= fseek(File, 0, SEEK_SET);
    if(SeekRes) {
        printf("Seek on file failed \n");
        return 1;
    }
    
    u8 *Buffer = malloc(FileSize);  // intentional leak, program lifetime
    if(!Buffer) {
        printf("Failed to alloc buffer\n");
        return 1;
    }
    
    if(fread(Buffer, 1, FileSize, File) != FileSize) {
        printf("Failed to read file\n");
        return 1;
    }
    
    switch(Mode) {
        case mode_pcx: {
            pcx_hdr *Hdr = (pcx_hdr *)Buffer;
            u8 *RLE = (u8 *)(Hdr+1);
            
            int w = Hdr->Xmax+1;
            int h = Hdr->Ymax+1;
            u8 *Img = malloc(w*h); // intentional leak, program lifetime
            if(!Img) {
                printf("Failed to alloc img\n");
                return 1;
            }
            
            {
                int TileW = w/8;
                int TileMemSize = 8*8;
                
                int x = 0;
                int y = 0;
                int i = 0;
                while(y < h) {
                    int ScanBegini = i;
                    while(x < w) {
                        u8 RunLength = 1;
                        if(RLE[i] >= 0xC0) {
                            RunLength = RLE[i++]-0xC0;
                        }
                        u8 Index = RLE[i++];
                        int TileY = y/8;
                        int InTileY = y%8;
                        pforj(RunLength) {
                            int TileX = x/8;
                            int InTileX = x%8;
                            Img[(TileY*TileW+TileX)*TileMemSize+(InTileY*8+InTileX)] = Index;
                            x++;
                        }
                    }
                    y++;
                    x=0;
                }
            }
            
            const char HdrBegin1[] = "// image atlas for gep -- ";
            const char HdrBegin2[] = "\nstatic const u8 ";
            const char HdrBegin3[] = "[] = {";
            i64 InputFileStrSize = strlen(InputFileStr);
            i64 VarNameStrSize = strlen(VarNameStr);
            i64 HdrBeginSize = ArraySize(HdrBegin1)-1+ArraySize(HdrBegin2)-1+ArraySize(HdrBegin3)-1+InputFileStrSize+VarNameStrSize;
            const char HdrEnd[] = "};\n";
            i64 ImgLen = w*h;
            u64 ImgHdrSize = HdrBeginSize + ArraySize(HdrEnd)-1 + ImgLen*6;
            char *ImgHdr = malloc(ImgHdrSize);  // intentional leak, program lifetime
            if(!ImgHdr) {
                printf("Failed to alloc img\n");
                return 1;
            }
            
            memcpy(ImgHdr, HdrBegin1, ArraySize(HdrBegin1)-1);
            i64 ImgHdrOffset = ArraySize(HdrBegin1)-1;
            memcpy(ImgHdr+ImgHdrOffset, InputFileStr, InputFileStrSize);
            ImgHdrOffset+= InputFileStrSize;
            memcpy(ImgHdr+ImgHdrOffset, HdrBegin2, ArraySize(HdrBegin2)-1);
            ImgHdrOffset+= ArraySize(HdrBegin2)-1;
            memcpy(ImgHdr+ImgHdrOffset, VarNameStr, VarNameStrSize);
            ImgHdrOffset+= VarNameStrSize;
            memcpy(ImgHdr+ImgHdrOffset, HdrBegin3, ArraySize(HdrBegin3)-1);
            ImgHdrOffset+= ArraySize(HdrBegin3)-1;
            
            static const char HexLookup[] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
            const char ItemTemplate[] = "0xCC, ";
            i64 ItemTemplateLen = ArraySize(ItemTemplate)-1;
            pfori(ImgLen) {
                i64 Pos = ImgHdrOffset+i*ItemTemplateLen;
                memcpy(ImgHdr+Pos, ItemTemplate, ItemTemplateLen);
                ImgHdr[Pos+2] = HexLookup[(Img[i]&0xF0)>>4];
                ImgHdr[Pos+3] = HexLookup[Img[i]&0x0F];
            }
            ImgHdrOffset += ImgLen*6;
            memcpy(ImgHdr+ImgHdrOffset, HdrEnd, ArraySize(HdrEnd)-1);
            FILE *OutFile = fopen(OutputFileStr, "wb");
            if(!OutFile) {
                printf("Failed to open out file \n");
                return 1;
            }
            
            if(fwrite(ImgHdr, 1, ImgHdrSize, OutFile) != ImgHdrSize) {
                printf("Failed to write out file\n");
                return 1;
            }
            
            if(fflush(OutFile)) {
                printf("Failed to fflush out file\n");
                return 1;
            }
            if(fclose(OutFile)) {
                printf("Failed to close out file\n");
                return 1;
            }
            printf("Atlas pack success!\n");
        } break;
        case mode_csv: {
            map Map = parse_csv((char *)Buffer, FileSize);
            FILE *OutFile = fopen(OutputFileStr, "wb");
            if(!OutFile) {
                printf("Failed to open out file \n");
                return 1;
            }
            
            const char Hdr1[] = "// Tilemap for gep\n";
            if(fprintf(OutFile, Hdr1) != ArrayLength(Hdr1)-1) {
                printf("Failed to write hdr1\n");
                exit(1);
            }
            
            const char DefineStr[] = "#define %s_Width %i\n#define %s_Height %i\n";
            i64 PrintSize = snprintf(0, 0, DefineStr, VarNameStr, Map.Width, VarNameStr, Map.Height);
            if(fprintf(OutFile, DefineStr, VarNameStr, Map.Width, VarNameStr, Map.Height) != PrintSize) {
                printf("Failed to write hdr2\n");
                exit(1);
            }
            
            const char Hdr2[] = "static u16 %s[] = { \n";
            PrintSize = snprintf(0, 0, Hdr2, VarNameStr);
            if(fprintf(OutFile, Hdr2, VarNameStr) != PrintSize) {
                printf("Failed to write hdr2\n");
                exit(1);
            }
            
            i64 NumTotal = Map.Width*Map.Height;
            pfory(Map.Height) {
                pforx(Map.Width) {
                    const char ValStr[] = "%lli, ";
                    PrintSize = snprintf(0, 0, ValStr, Map.Data[Map.Width*y+x]);
                    if(fprintf(OutFile, ValStr, Map.Data[Map.Width*y+x]) != PrintSize) {
                        printf("Failed to write val #%lli\n", Map.Width*y+x);
                        exit(1);
                    }
                }
                if(fprintf(OutFile, "\n") != 1) {
                    printf("Failed to write value newline\n");
                    exit(1);
                }
            }
            
            const char Tail[] = "};\n";
            PrintSize = snprintf(0, 0, Tail);
            if(fprintf(OutFile, Tail) != PrintSize) {
                printf("Failed to write Tail\n");
                exit(1);
            }
            
            if(fflush(OutFile)) {
                printf("Failed to fflush out file\n");
                return 1;
            }
            if(fclose(OutFile)) {
                printf("Failed to close out file\n");
                return 1;
            }
            printf("Map pack success!\n");
        } break;
        default: {
            printf("given unhandled file mode\n");
            return 1;
        }
    }
    
    return 0;
}