// serialization code notes:
/*
- As long as new serialize() members are added to the back, and none removed, 
old files can be loaded without a version change.
- If struct members get removed, the serialize() should stay in place and use 
a dummy value instead (type){0}. For readability, keep the name of the 
serialized member as a comment
- To fully remove serialize()d data, do a version bump. Load with the 
appropriate version for the file and write with the most recent version and 
everything should work out fine.
   --* This means only the function that reads the file needs to deal with 
different versions, the function that writes can just write the latest
- To make this simpler, append a version number to the names of your serialize 
functions
- use the serialize_generic_header_base() at the beginning of the file.
- each variable length block of data needs its own sub-header describing at 
least the size of the data objects and their number.

 Explanation and Examples:
In the following function we are serializing a basic game object. One of its 
members has been removed and is no longer used. The function name ends with 
the number 1 to denote that it is the first version
*/
#if 0
int serialize_object1(sctx *Ctx, object *Object) {
    serialize(Ctx, int, Object->x);
    serialize(Ctx, int, Object->y);
    serialize(Ctx, b32, Object->UsesCollision);
    serialize(Ctx, b32, (int){0}); // was Object->HasBanana
    serialize(Ctx, obj_type, Object->Type);
    
    return Ctx->SerializeSize;
}
#endif
/*
To use this serialization we allocate a context on the stack and 
zero-initialize it (this is important).
The functions ssize(), sread(), and swrite() are used to configure the context 
for each of these operation modes, and to reset it between serialization 
function calls. Always call the appropriate function before a serialization.
*/
#if 0
...
sctx Ctx = {0};
swrite(&Ctx)
serialize_object1(&Ctx, Object);
...
#endif
/*
This can also be made a little more compact by calling the context preparation 
function as the first argument.
*/
#if 0
...
sctx Ctx = {0};
serialize_object1(swrite(&Ctx), Object);
...
#endif
/*
To serialize and de-serialize a whole file, do something like the following:
*/

#if 0
typedef struct {
    int x;
    int y;
    b32 UsesCollision;
    obj_type Type;
} object;

int serialize_object_header1(sctx *Ctx, int *NumObjects, int *ObjectSize) {
    serialize(Ctx, int, *NumObjects);
    serialize(Ctx, int, *ObjectSize);
    
    return Ctx->SerializeSize;
}

int serialize_object1(sctx *Ctx, object *Object) {
    serialize(Ctx, int, Object->x);
    serialize(Ctx, int, Object->y);
    serialize(Ctx, b32, Object->UsesCollision);
    serialize(Ctx, b32, (int){0}); // was Object->HasBanana
    serialize(Ctx, obj_type, Object->Type);
    
    return Ctx->SerializeSize;
}

b32 object_file_write(object *Objects, int NumObjects) {
    u64 Magic = *(u64*)"OBJECTDB";
    int Version = 1;
    
    sctx Ctx = {0};
    
    FILE *File = fopen("obj.db.tmp", "wb");
    if(!File) return 0;
    
    // get sizes
    int HeaderBaseSize = serialize_generic_header(ssize(&Ctx), 0,0,0);
    int HeaderObjectSize = serialize_object_header1(ssize(&Ctx),0,0);
    int HeaderSize = HeaderBaseSize+HeaderObjectSize;
    int ObjectSize = serialize_object1(ssize(&Ctx), 0);
    
    // write base header
    if(serialize_generic_header(swrite(&Ctx), &Magic, &Version, &HeaderSize)==-1) goto err;
    if(fwrite(Ctx.Buffer, HeaderBaseSize, 1, File)!=1) goto err;
    
    // write object header
    if(serialize_object_header1(swrite(&Ctx), &NumObjects, &ObjectSize)==-1) goto err;
    if(fwrite(Ctx.Buffer, HeaderObjectSize, 1, File)!=1) goto err;
    
    // write objects
    pfori32(NumObjects) {
        if(serialize_object1(swrite(&Ctx), &Objects[i])==-1) goto err;
        if(fwrite(Ctx.Buffer, ObjectSize, 1, File)!=1) goto err;
    }
    if(fclose(File)!=0) goto err;
    
    if(tmp_file_copy("obj.db.tmp", "obj.db")==0) return 0;
    
    return 1;
    err:
    fclose(File);
    return 0;
}

b32 object_file_read(object *Objects, int MaxObjects, int *NumObjects) {
    u64 Magic = 0;
    int Version = 0;
    int HeaderSize = 0;
    
    sctx Ctx = {0};
    
    FILE *File = fopen("object.db", "rb");
    if(!File) return 0;
    
    // read base header
    int HeaderBaseSize = serialize_generic_header(ssize(&Ctx), 0,0,0);
    if(fread(Ctx.Buffer, HeaderBaseSize, 1, File)!=1) goto err;
    if(serialize_generic_header(sread(&Ctx, HeaderBaseSize), &Magic, &Version, &HeaderSize)==-1) goto err;
    // validate file magic value
    u64 ExpectedMagic = *(u64*)"OBJECTDB";
    if(ExpectedMagic != Magic) goto err;
    switch(Version) {
        case 1: {
            int ObjectSize = 0;
            // read object header
            int HeaderRestSize = HeaderSize-HeaderBaseSize;
            if(fread(Ctx.Buffer, HeaderRestSize, 1, File)!=1) goto err;
            if(serialize_object_header1(sread(&Ctx, HeaderRestSize), NumObjects, &ObjectSize)==-1) goto err;
            // Validate object header values
            if(*NumObjects > MaxObjs) goto err;
            
            // read objects
            pfori32(*NumObjects) {
                if(fread(Ctx.Buffer, ObjectSize, 1, File)!=1) goto err;
                object *o = &Objects[i];
                *o = (object){0};
                if(serialize_object1(sread(&Ctx, ObjectSize), o) == -1) goto err;
            }
        } break;
        default: goto err;
    }
    fclose(File);
    
    return 1;
    err:
    fclose(File);
    return 0;
}
#endif
/*
###############################################################################
############################# END OF DOCUMENTATION ############################
###############################################################################
*/

typedef enum {
    smode_size_only,
    smode_write,
    smode_read,
} serialize_mode;

#define SCtxBufferSize 1024
typedef struct {
    char Buffer[SCtxBufferSize]; // The buffer used for (de-)serialization
    serialize_mode Mode;
    int MaxReadSize; // When in mode_read, use this to limit reading up to the object size from the respective header
    int SerializeSize; // The size of the data actually serialized. Used for nesting serialization functions. NOTE: Needs to be reset to zero before calling a top-level serialization function (by default, this is done in ssize(), sread() and swrite())
} sctx;

#define serialize(Ctx, Type, Val) \
if((Ctx)->Mode == smode_read && (Ctx)->MaxReadSize > 0 && (Ctx)->SerializeSize+sizeof(Type) > (Ctx)->MaxReadSize) return (Ctx)->SerializeSize+sizeof(Type); \
if((Ctx)->Mode != smode_size_only && (Ctx)->SerializeSize+sizeof(Type) > ArraySize((Ctx)->Buffer)) return -1; \
if((Ctx)->Mode == smode_write) {*((Type *)((Ctx)->Buffer+(Ctx)->SerializeSize)) = (Val);} \
else if((Ctx)->Mode == smode_read){(Val) = *((Type *)((Ctx)->Buffer+(Ctx)->SerializeSize));} \
(Ctx)->SerializeSize+=sizeof(Type);

sctx *ssize(sctx *Ctx) {
    Ctx->Mode = smode_size_only;
    Ctx->SerializeSize = 0;
    return Ctx;
}

// MaxReadSize is the object size as read from the header. If the data object in the file is smaller than the object in the current serialization function, this stops the read after the appropriate number of bytes.
sctx *sread(sctx *Ctx, int MaxReadSize) {
    Ctx->Mode = smode_read;
    Ctx->SerializeSize = 0;
    Ctx->MaxReadSize = MaxReadSize;
    return Ctx;
}

sctx *swrite(sctx *Ctx) {
    Ctx->Mode = smode_write;
    Ctx->SerializeSize = 0;
    return Ctx;
}

#ifdef GEP_SERIALIZE_USE_STDIO
#include<stdio.h>
b32 tmp_file_copy(const char *TmpFileName, const char *FinishedFileName) {
    remove(FinishedFileName); // ignore if it doesn't work
    if(rename(TmpFileName, FinishedFileName) != 0) {
        return 0;
    }
    remove(TmpFileName); // ignore if this doesn't work, we don't really care.
    return 1;
}
#endif

int serialize_generic_header(sctx *Ctx, u64 *Magic, int *Version, int *HeaderSize) {
    serialize(Ctx, u64, *Magic);
    serialize(Ctx, int, *Version);
    serialize(Ctx, int, *HeaderSize);
    
    return Ctx->SerializeSize;
}
