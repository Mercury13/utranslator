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
        virtual void save(pugi::xml_node& root) const = 0;

        // Boilerplate
        virtual ~Place() = default;
        bool operator == (const Place& x) const = default;
    };

    using EvLoadPlace = std::unique_ptr<Place>(*)(const pugi::xml_node& node);

    class FilePlace : public Place
    {
    public:
        FilePlace(const std::filesystem::path& aPath);
        FilePlace(std::filesystem::path&& aPath);
        bool operator == (const FilePlace& x) const = default;
        const std::filesystem::path path() const { return fPath; }

        std::unique_ptr<FilePlace> tryRead(const pugi::xml_node& node);

        // Place
        std::wstring shortName() const override;
        std::wstring auxName() const override;
        bool eq(const Place& aPlace) const override;
        void save(pugi::xml_node& root) const override;
        static std::unique_ptr<FilePlace> tryLoadSpec(const pugi::xml_node& node);
        static std::unique_ptr<Place> tryLoad(const pugi::xml_node& node)
            { return tryLoadSpec(node); }
    private:
        std::filesystem::path fPath;
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

        /// Pushes file, the main type of document
        void pushFile(std::filesystem::path fname);

        /// Adds an item to end if there’s room
        /// @return [+] changed smth (place has data, history is not full)
        bool silentAdd(std::shared_ptr<Place> x);

        /// @return [+] first item is x
        bool firstIs(const Place& x);

        size_t size() const { return sz; }
        const std::shared_ptr<Place>& operator [] (size_t i) const { return d.at(i); }
        [[nodiscard]] bool isEmpty() const { return (sz == 0); }
        [[nodiscard]] bool isFull() const { return (sz >= SIZE); }

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

        /// @param [in] root node ABOVE
        void save(pugi::xml_node& root, const char* name) const;
        /// @param [in] node THIS node
        void silentLoad(pugi::xml_node& node,
                  std::initializer_list<EvLoadPlace> loadPlaces =
                        { FilePlace::tryLoad });
        void load(pugi::xml_node& node,
                  std::initializer_list<EvLoadPlace> loadPlaces =
                        { FilePlace::tryLoad });
    private:
        Ar d;
        size_t sz = 0;
        Listener* lstn = nullptr;
    };

}   // namespace hist
