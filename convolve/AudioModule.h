//
//  AudioModule.hpp
//  convolve
//
//  Created by Graham Barab on 5/8/16.
//  Copyright © 2016 Graham Barab. All rights reserved.
//

#ifndef AudioModule_hpp
#define AudioModule_hpp

namespace Gmb {
    typedef void(*renderProc)(double *out, unsigned int n, void *module);

    class AudioModule {
    protected:
        unsigned int sysBufSize;
        AudioModule *_upstream;
    public:
        friend renderProc;
        AudioModule(renderProc renderProcedure, AudioModule *upstream, unsigned int bufsize) {_render = renderProcedure; sysBufSize = bufsize; _upstream = upstream;};
        virtual renderProc addrOfRenderProc() {return _render;};
        void render(double *out, unsigned int n) {_render(out, n, this);};
        void setBufSize(unsigned int newBufSize) {sysBufSize = newBufSize;};
        AudioModule *upstream() const {return _upstream;};
        renderProc _render;
    };
}

#endif /* AudioModule_hpp */
