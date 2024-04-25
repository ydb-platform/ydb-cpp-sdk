JSON writer with no external dependencies, producing output
where HTML special characters are always escaped.

Use it like this:

<<<<<<< HEAD
<<<<<<< HEAD
    #include <ydb-cpp-sdk/library/json/writer/json.h>
=======
    #include <src/library/json/writer/json.h>
<<<<<<< HEAD
>>>>>>> ed2145fb77 (Moved SDK code to src (#149))
=======
>>>>>>> 64d9ce2d94 (Moved SDK code to src (#149))
>>>>>>> 02ff417676 (Moved SDK code to src (#149))
=======
    #include <ydb-cpp-sdk/library/json/writer/json.h>
>>>>>>> origin/main
    ...

    NJsonWriter::TBuf json;
    json.BeginList()
        .WriteString("<script>")
        .EndList();
    std::cout << json.Str(); // output: ["\u003Cscript\u003E"]

For compatibility with legacy formats where object keys
are not quoted, use CompatWriteKeyWithoutQuotes:
    
    NJsonWriter::TBuf json;
    json.BeginObject()
        .CompatWriteKeyWithoutQuotes("r").WriteInt(1)
        .CompatWriteKeyWithoutQuotes("n").WriteInt(0)
    .EndObject();
    std::cout << json.Str(); // output: {r:1,n:0}
