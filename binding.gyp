{
  "targets": [
    {
      "include_dirs": [
        "<!(node -e \"require('nan')\")"
      ],
      "libraries": [ "-lavcodec", "-lavutil", "-lavformat", "-lavresample", "-lswscale", "-lm" ],
      "target_name": "addon",
      "sources": [ "src/mpeg/module.cpp", "src/mpeg/mpeg.cpp" ]
    }
  ]
}
