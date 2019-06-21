#include "ast/ast.h"

namespace sorbet::autogen {

const u4 NONE_ID = (u4)-1;

struct ParsedFile;
struct Reference;
struct Definition;

struct DefinitionRef {
    u4 _id;

    DefinitionRef() : _id(NONE_ID){};
    DefinitionRef(u4 id) : _id(id) {}

    u4 id() {
        return _id;
    }

    bool exists() {
        return _id != NONE_ID;
    }

    Definition &data(ParsedFile &pf);
};

struct ReferenceRef {
    u4 _id;
    ReferenceRef() : _id(NONE_ID){};
    ReferenceRef(u4 id) : _id(id) {}

    u4 id() {
        return _id;
    }

    bool exists() {
        return _id != NONE_ID;
    }

    Reference &data(ParsedFile &pf);
};

struct Definition {
    enum Type { Module, Class, Casgn, Alias };

    DefinitionRef id;

    Type type;
    bool defines_behavior;
    bool is_empty;

    ReferenceRef parent_ref;
    ReferenceRef aliased_ref;
    ReferenceRef defining_ref;
};

struct NamedDefinition {
    Definition def;
    std::string name;
    std::vector<core::NameRef> nameParts;
    std::vector<core::NameRef> parentName;
    std::vector<core::NameRef> requires;
    core::FileRef fileRef;

    std::string_view toString(core::Context ctx) const;
};

struct AutoloaderConfig { // TODO dynamic loading
    UnorderedSet<std::string> topLevelNamespaces{"Opus", "Critic", "Chalk", "T", "Foo", "Yabba"}; // TODO TODO
    std::string_view rootDir = "autoloader";

    bool include(core::Context, const NamedDefinition &) const;
};

class DefTree {
public:
    std::string name;                                             // TODO switch to refs
    UnorderedMap<std::string, std::unique_ptr<DefTree>> children; // TODO switch to refs
    std::vector<NamedDefinition> namedDefs;
    std::vector<core::NameRef> nameParts;

    void addDef(core::Context, const AutoloaderConfig &, const NamedDefinition &);
    void prettyPrint(core::Context ctx, int level = 0);

    void writeAutoloads(core::Context ctx, std::string &path);
    std::string autoloads(core::Context ctx);

    std::string path(core::Context ctx);

private:
    void predeclare(core::Context ctx, std::string_view fullName, fmt::memory_buffer &buf);
    void requires(core::Context ctx, fmt::memory_buffer &buf);
};

struct Reference {
    ReferenceRef id;

    DefinitionRef scope;
    std::vector<core::NameRef> name;
    std::vector<DefinitionRef> nesting;
    std::vector<core::NameRef> resolved;

    core::Loc loc;
    core::Loc definitionLoc;
    bool is_resolved_statically;
    bool is_defining_ref;

    DefinitionRef parent_of;
};

struct ParsedFile {
    ast::ParsedFile tree;
    u4 cksum;
    std::string path;
    std::vector<Definition> defs;
    std::vector<Reference> refs;
    std::vector<core::NameRef> requires;

    std::string toString(core::Context ctx);
    std::string toMsgpack(core::Context ctx, int version);
    void classlist(core::Context ctx, std::vector<std::string> &out);
    NamedDefinition toNamed(core::Context ctx, DefinitionRef def);

private:
    std::vector<core::NameRef> showFullName(core::Context ctx, DefinitionRef id);
    friend class MsgpackWriter;
};

class Autogen final {
public:
    static ParsedFile generate(core::Context ctx, ast::ParsedFile tree);
    Autogen() = delete;
};
} // namespace sorbet::autogen
