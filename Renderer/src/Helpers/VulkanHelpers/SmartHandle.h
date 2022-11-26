#pragma once



template <typename HandleType>
class SmartHandle
{
public:
    SmartHandle() = default;

    SmartHandle(SmartHandle const&& rhs)
    {
        mHandle = rhs.mHandle;
    }

    ~SmartHandle();

private:
    HandleType mHandle;
};

