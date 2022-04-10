#pragma once

// STL
#include <memory>
#include <filesystem>

// Libs
#include "u_Vector.h"


namespace pugi {
    class xml_node;
}

namespace hist {

    class Listener     // interface
    {
    public:
        virtual void historyChanged() = 0;
        virtual ~Listener() = default;
    };

    class Place         // interface
    {
    public:
        /// @return [+] two places are equal
        virtual bool eq(const Place& x) const = 0;
        virtual std::wstring shortName() const = 0;
        virtual std::wstring auxName() const = 0;

        // Boilerplate
        virtual ~Place() = default;
        bool operator == (const Place& x) const = default;
    };

    class History
    {
    public:
        using Sp = std::shared_ptr<Place>;
        static constexpr int SIZE = 10;
    private:
        using Ar = Fix1d<Sp, SIZE>;
    public:
        /// A few listening functions
        void setListener(Listener* x) { lstn = x; }
        void setListener(Listener& x) { lstn = &x; }
        void notify();

        /// Adds a new place, probably deleting the last one,
        ///    calls reflector if smth changed
        /// @return [+] changed smth
        bool push(const std::shared_ptr<Place>& x);

        /// Silently adds a new place, probably deleting the last one
        /// @return [+] changed smth
        bool silentPush(const std::shared_ptr<Place>& x);

        /// Adds an item to end if there’s room
        void silentAdd(const std::shared_ptr<Place>& x);

        /// @return [+] first item is x
        bool firstIs(const Place& x);

        size_t size() const { return sz; }
        const std::shared_ptr<Place>& operator [] (size_t i) const { return d.at(i); }
        [[nodiscard]] bool isEmpty() const { return (sz == 0); }
        [[nodiscard]] bool isFull() const { return (sz < SIZE); }

        /// Silently makes i’th item the 1st
        /// @return [+] changed smth
        bool silentBump(size_t i);

        /// Makes i’th item the 1st, calling reflector if smth changed
        /// @return [+] changed smth
        bool bump(size_t i);

        using iterator = Ar::const_iterator;
        iterator begin() const { return d.begin(); }
        iterator end() const { return d.begin() + sz; }
        iterator cbegin() const { return d.begin(); }
        iterator cend() const { return d.begin() + sz; }

        /// @return [+] changed smth
        bool silentClear();
        bool clear();
    private:
        Ar d;
        size_t sz = 0;
        Listener* lstn = nullptr;
    };

    class FilePlace : public Place
    {
    public:
        FilePlace(std::filesystem::path& aPath) : fPath(aPath) {}
        bool operator == (const FilePlace& x) const = default;
        const std::filesystem::path path() const { return fPath; }

        std::unique_ptr<FilePlace> tryRead(const pugi::xml_node& node);

        // Place
        std::wstring shortName() const override;
        std::wstring auxName() const override;
        bool eq(const Place& aPlace) const override;
    private:
        std::filesystem::path fPath;
    };

}   // namespace hist