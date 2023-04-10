{
  "targets": [
    {
      "target_name": "sim8086",
      "cflags!": [ "-fno-exceptions" ],
      "cflags_cc!": [ "-fno-exceptions" ],
      "sources": [ "sim8086_addon.cc" ],
      "conditions":[
        ['OS=="linux"', {'libraries': ['/usr/local/lib/libsim86.so']}]],
      "include_dirs": [
        "<!@(node -p \"require('node-addon-api').include\")"
      ],
      'defines': [ 'NAPI_DISABLE_CPP_EXCEPTIONS' ],
    }
  ]
}
