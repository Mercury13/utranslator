#include "TrSave.h"

#include "pugixml.hpp"

namespace {

    /// Write text in tag
    /// @param root   an upper element
    /// @param name   tag name
    /// @param text   text itself
    void writeTextInTag(
            pugi::xml_node root,
            const char* name,
            std::u8string_view text,
            tr::WrCache& cache)
    {
        const char8_t* data = text.data();
        const char8_t* end = data + text.length();

        auto node = root.append_child(name);

        auto pBreak = std::find(data, end, '\n');
        if (pBreak == end) {
            // Simple write
            auto tx = node.append_child(pugi::node_pcdata);
                tx.set_value(str::toC(data));
        } else {
            // Paragraphs
            do {
                auto nextLine = node.append_child("p");
                if (pBreak != data) {
                    // Even empty pcdata kills <p /> → only when non-empty
                    auto tx = nextLine.append_child(pugi::node_pcdata);
                    tx.set_value(cache.ntsC(data, pBreak));
                }
                data = pBreak + 1;
                pBreak = std::find(data, end, '\n');
            } while (pBreak != end);
            // Last line
            auto lastLine = node.append_child("p");
            auto tx = lastLine.append_child(pugi::node_pcdata);
            tx.set_value(str::toC(data));
        }
    }

    /// Write text in tag if the text is not empty (for comments)
    void writeTextInTagIf(
            pugi::xml_node root,
            const char* name,
            const std::u8string& text,
            tr::WrCache& cache)
    {
        if (!text.empty())
            writeTextInTag(root, name, text, cache);
    }

    /// Write text in tag if the optional is not empty (for known text, translation)
    void writeTextInTagOpt(
            pugi::xml_node root,
            const char* name,
            const std::optional<std::u8string_view>& text,
            tr::WrCache& cache)
    {
        if (text.has_value())
            writeTextInTag(root, name, *text, cache);
    }

    std::u8string parseTextInTagOld(pugi::xml_node tag)
    {
        std::u8string r;
        for (auto v : tag.children()) {
            switch (v.type()) {
            case pugi::node_pcdata:
            case pugi::node_cdata:
                r += str::toU8sv(v.value());
                break;
            case pugi::node_element:
                if (strcmp(v.name(), "br") == 0)
                    r += '\n';
                break;
            default: ;
            }
        }
        return r;
    }

    std::u8string parseTextInTag(pugi::xml_node tag)
    {
        if (auto para = tag.child("p")) {
            std::u8string r;
            while (true) {
                r += parseTextInTagOld(para);
                para = para.next_sibling("p");
                if (!para)
                    break;
                r += '\n';
            }
            return r;
        } else {
            return parseTextInTagOld(tag);
        }
    }

    std::u8string readTextInTag(
            pugi::xml_node root, const char* name)
    {
        if (auto tag = root.child(name))
            return parseTextInTag(tag);
        return {};
    }

    std::optional<std::u8string> readTextInTagOpt(
            pugi::xml_node root, const char* name)
    {
        if (auto tag = root.child(name))
            return parseTextInTag(tag);
        return std::nullopt;
    }

    void writeImportersAuthorsComment(
            const tr::Entity& entity,
            pugi::xml_node& node, tr::WrCache& c)
    {
        /// @todo [urgent, standalone save] delete completely
        writeTextInTagIf(node, "im-cmt", entity.comm.importers, c);
        writeTextInTagIf(node, "au-cmt", entity.comm.authors, c);
    }

    void writeTranslatorsComment(
            const tr::Entity& entity,
            pugi::xml_node& node, tr::WrCache& c)
    {
        if (c.info.isTranslation()) {
            writeTextInTagIf(node, "tr-cmt", entity.comm.translators, c);
        }
    }

    void writeComments(
            const tr::Entity& entity,
            pugi::xml_node& node, tr::WrCache& c)
    {
        writeImportersAuthorsComment(entity, node, c);
        writeTranslatorsComment(entity, node, c);
    }

    void writeFormat(pugi::xml_node parent, tf::FileFormat* format)
    {
        if (format) {
            auto hFormat = parent.append_child("format");
            hFormat.append_attribute("name") = format->proto().techName().data();  // Tech names are const, so OK
            format->save(hFormat);
        }
    }

    class FileListener final : public tr::ConstTraverseListener
    {
    public:
        FileListener(tr::WrCache& aCache, pugi::xml_node root);
        void onText(const std::shared_ptr<const tr::Text>&) override;
        void onEnterGroup(const std::shared_ptr<const tr::VirtualGroup>&) override;
        void onLeaveGroup(const std::shared_ptr<const tr::VirtualGroup>&) override;
    private:
        tr::WrCache& cache;
        struct Entry {
            pugi::xml_node node;
        };
        std::vector<Entry> stack;
    };

    FileListener::FileListener(tr::WrCache& aCache, pugi::xml_node root)
        : cache(aCache)
    {
        stack.emplace_back(root);
    }

    void FileListener::onEnterGroup(const std::shared_ptr<const tr::VirtualGroup>& g)
    {
        // Create node
        auto node = stack.back().node.append_child("group");
        stack.emplace_back(node);
        // Go!
        node.append_attribute("id") = str::toC(g->id);
        if (auto gg = dynamic_cast<const tr::Group*>(g.get());
                gg && gg->sync) {
            auto hSync = node.append_child("sync");
            hSync.append_attribute("text-owner") =
                    tf::textOwnerNames[static_cast<int>(gg->sync.info.textOwner)];
            hSync.append_attribute("fname") =
                   str::toC(cache.toRelPath(gg->sync.absPath).u8string());
           writeFormat(hSync, gg->sync.format.get());
       }
       writeComments(*g, node, cache);
    }

    void FileListener::onLeaveGroup(const std::shared_ptr<const tr::VirtualGroup>&)
    {
        stack.pop_back();
    }

    void FileListener::onText(const std::shared_ptr<const tr::Text>& text)
    {
        auto node = stack.back().node.append_child("text");
            node.append_attribute("id") = str::toC(text->id);
            if (text->tr.forceAttention) {
                node.append_attribute("force-attention") = true;
            }
        writeTextInTag(node, "orig", text->tr.original, cache);
        writeImportersAuthorsComment(*text, node, cache);
        if (cache.info.isTranslation()) {
            writeTextInTagOpt(node, "known-orig", text->tr.knownOriginal.active(), cache);
            writeTextInTagOpt(node, "transl", text->tr.translation, cache);
            writeTranslatorsComment(*text, node, cache);
        }
    }

    void writeFileToXml(
            const tr::File& file, pugi::xml_node& root, tr::WrCache& cache)
    {
        auto node = root.append_child("file");
            node.append_attribute("name") = str::toC(file.id);
            node.append_attribute("idless") = file.info.isIdless;
            if (!file.info.origPath.empty())
                node.append_attribute("orig-path") = str::toC(file.info.origPath.u8string());
            if (!file.info.translPath.empty())
                node.append_attribute("transl-path") = str::toC(file.info.translPath.u8string());
            writeFormat(node, file.info.format.get());
            writeComments(file, node, cache);
        FileListener fl(cache, node);
        file.traverse(fl, tr::WalkOrder::EXACT, tr::EnterMe::NO);
    }

    void writeProjectToXml(
            const tr::Project& project,
            pugi::xml_node& doc,
            const std::filesystem::path& basePath)
    {
        auto root = doc.append_child("ut");
        /// @todo [urgent, standalone save] move prjTypeNames here
        root.append_attribute("type") = tr::prjTypeNames[project.info.type];
        tr::WrCache c(project.info, basePath);
        auto nodeInfo = root.append_child("info");
            auto nodeOrig = nodeInfo.append_child("orig");
                nodeOrig.append_attribute("lang") = project.info.orig.lang.c_str();
                if (project.info.hasOriginalPath()) {
                    if (!project.info.orig.absPath.empty()) {
                        auto relPath = c.toRelPath(project.info.orig.absPath);
                        nodeOrig.append_attribute("fname") = str::toC(relPath.u8string());
                    }
                }
        if (project.info.canHaveReference() && !project.info.ref.absPath.empty()) {
            auto nodeRef = nodeInfo.append_child("ref");
                auto relPath = c.toRelPath(project.info.ref.absPath);
                nodeRef.append_attribute("fname") = str::toC(relPath.u8string());
        }
        if (project.info.isTranslation()) {
            auto nodeTransl = nodeInfo.append_child("transl");
                nodeTransl.append_attribute("lang") = project.info.transl.lang.c_str();
                if (project.info.isFullTranslation()) {
                    bool hasPseudoloc = project.info.transl.pseudoloc.isOn();
                    nodeTransl.append_attribute("pseudoloc") = hasPseudoloc;
                }
        }
        for (auto& file : project.files) {
            writeFileToXml(*file, root, c);
        }
    }

}

void sav::saveCopy(const tr::Project& project, const std::filesystem::path& fname)
{
    pugi::xml_document doc;
    auto declaration = doc.append_child(pugi::node_declaration);
        declaration.append_attribute("version") = "1.0";
        declaration.append_attribute("encoding") = "utf-8";
    writeProjectToXml(project, doc, fname.parent_path());
    auto r = doc.save_file(fname.c_str(), " ",
            pugi::format_indent | pugi::format_write_bom | pugi::format_save_file_text);
    if (!r)
        throw std::logic_error("Cannot save file");
}

void sav::save(tr::Project& project)
{
    saveCopy(project, project.fname);
    project.unmodify(Forced::YES);
}

void sav::save(tr::Project& project, const std::filesystem::path& fname)
{
    saveCopy(project, fname);
    project.fname = fname;
    project.unmodify(Forced::YES);
}


///// Update ///////////////////////////////////////////////////////////////////

namespace {

    tr::UpdateInfo updateData_FullTransl(
            tr::Project& project, tr::TrashMode mode)
    {
        auto tempPrj = tr::Project::make();
        tempPrj->load(project.info.orig.absPath);
        // Info and fname are left intact
        std::swap(project.files, tempPrj->files);
        // Copy original language
        if (!tempPrj->info.orig.lang.empty())
            project.info.orig.lang = tempPrj->info.orig.lang;
        project.removeTranslChannel();    // Do not forget, we swapped!
        tr::StealContext ctx {
            .orig = tf::StealOrig::KEEP_WARN,
            .trash = (mode == tr::TrashMode::FILL) ? &project.trash : nullptr,
        };
        size_t origTrashSize = project.trash.size();
        auto r = project.stealDataFrom(*tempPrj, ctx);
        // Stats will always be funked up!
        project.updateParents();
        if (mode == tr::TrashMode::FILL) {
            project.suggestTrash(origTrashSize);
        }
        project.stats(tr::StatsMode::ALL_CHILDREN, tr::CascadeDropCache::NO);
        return r;
    }

}   // anon namespace


tr::UpdateInfo sav::updateData(
        tr::Project& project, tr::TrashMode trashMode)
{
    switch (project.info.type) {
    case tr::PrjType::ORIGINAL:
        return { .isOriginal = true };
    case tr::PrjType::FULL_TRANSL:
        return updateData_FullTransl(project, trashMode);
    }
    throw std::logic_error("[updateData] Strange project type");
}
