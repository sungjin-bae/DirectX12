#ifndef MYDIRECT_SOURCE_MYDXAPP_H
#define MYDIRECT_SOURCE_MYDXAPP_H

#include "Common/d3dApp.h"

class MyDXApp : public D3DApp
{
public:
    MyDXApp(HINSTANCE hInstance);
    virtual ~MyDXApp();

    virtual bool Initialize() override;

private:
    virtual void OnResize() override;
    virtual void Update(const GameTimer& gt) override;
    virtual void Draw(const GameTimer& gt) override;
};

#endif  // MYDIRECT_SOURCE_MYDXAPP_H
