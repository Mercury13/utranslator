#pragma once

#include <memory>
#include <atomic>

class Modifiable    // interface
{
public:
    virtual bool isModified() const = 0;
    virtual void modify() = 0;
    virtual void unmodify() = 0;
    virtual ~Modifiable() = default;
};


class SimpleModifiable : public Modifiable
{
public:
    void setListener(std::shared_ptr<Modifiable> aListener)
        { fListener = aListener; }
    void setStaticListener(Modifiable& aListener);
    bool isModified() const override { return fIsModified; }
    void modify() override;
    void unmodify() override;
private:
    std::atomic<bool> fIsModified = false;
    std::weak_ptr<Modifiable> fListener;
};
