#pragma once

class UConsole : public UObject
{
public:
};

class UGameViewportClient : public UObject
{
public:
    DEFINE_PROPERTY(UConsole*, ViewportConsole);
};

class UEngine : public UObject
{
public:
    DEFINE_PROPERTY(UGameViewportClient*, GameViewport);
};