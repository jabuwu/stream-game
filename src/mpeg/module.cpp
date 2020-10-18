#include <nan.h>
#include "mpeg.hpp"

Mpeg mpeg;
bool first = true;
unsigned char* rgb = (unsigned char*)malloc(_WIDTH * _HEIGHT * 3);

#include <iostream>
NAN_METHOD(addFrame) {
    if (first) {
        mpeg.create();
        first = false;
    }

    if (info.Length() > 0) {
        if (info[0]->IsUint8ClampedArray()) {
            auto clampedArray = info[0].As<v8::Uint8ClampedArray>();
            unsigned char *data = (unsigned char *)clampedArray->Buffer()->GetContents().Data();
            auto length = clampedArray->Length();
            int j = 0;
            for (unsigned int i = 0; i < length; ++i) {
                if (i % 4 != 3) {
                    rgb[j] = data[i];
                    j++;
                }
            }
        }
    }

    char* out;
    size_t len;
    mpeg.addFrame(rgb, &out, &len);

    //mpeg.close();
    //fclose(_file);

    info.GetReturnValue().Set(Nan::CopyBuffer(out, len).ToLocalChecked());
}
NAN_MODULE_INIT(Initialize) {
    NAN_EXPORT(target, addFrame);
}
NODE_MODULE(addon, Initialize);