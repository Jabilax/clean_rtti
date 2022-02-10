#include <string>
#include <fstream>
#include <iostream>
#include <cassert>
#include <set>
#include <map>
#include <vector>
#include <sstream>
#include <windows.h>
#include "parser.h"

#define print(what) std::cout << #what << " : " << what << "\n"
//#define is_equal(compare) [](const std::string & word) -> bool { return word == compare; }

namespace
{
    struct property
    {
        std::string name;
        std::vector<std::string> arguments;
        std::vector<std::string> numeric_arguments;
        std::vector<std::string> string_arguments;
        std::vector<std::string> type_arguments;

        bool is_function{ false };
        bool is_event{ false };
        bool attribute_not_visible{ false };
        bool attribute_not_resizeable{ false };
        bool attribute_not_serializable{ false };

        std::string default_value;
    };

    struct component
    {
        component(const std::string & name)
            : name{ name }
        {}

        std::string name;
        int container_index{ 0 };
        std::vector<std::string> dependencies;
        bool owner_knowledge{ false };

        bool operator<(const component & component) const { return component.name < name; }
    };

    struct resource
    {
        resource(const std::string & name)
            : name{ name }
        {}

        std::string name;
        std::vector<std::string> extensions;
        int container_index{ 0 };
    };

    struct container
    {
        container(const std::string& name)
            : name{ name }
        {}

        std::string name;
        std::string file;
        std::vector<property> properties;

        bool component{ false };
        bool manager{ false };
        bool resource{ false };
        std::string manager_extension{};

        bool free() { return !component && !resource; }
    };

    struct write_file
    {
        std::string pathname;
        std::string text;

        write_file & operator<< (const char * text_data) { text.append(text_data); return *this; }
        write_file & operator<< (const std::string & text_data) { text.append(text_data); return *this; }
        write_file & operator<< (const char text_data) { text.append(std::string(1, text_data)); return *this; }
    };

    struct deep_dependency
    {
        deep_dependency()
            : component{ nullptr }
        {}

        deep_dependency(component& component)
            : component{ &component } 
        {}

        std::vector<std::string> parents;
        component * component;
    };

    struct enum_data
    {
        enum_data(const std::string& name)
            : name{ name }
        {}

        std::vector<std::string> elements;
        std::string name;
        std::string file;
    };

    struct event
    {
        std::map<std::string, std::vector<std::string>> component_funtions;
    };

    std::vector<component> components;
    std::vector<resource> resources;
    std::vector<write_file> write_files;
    std::vector<container> containers;
    std::vector<enum_data> enums;
    std::vector<std::string> cpp_files;
    std::map<std::string, std::vector<component>> direct_dependents; // all components which include the given one directly
    std::map<std::string, std::vector<component>> dependants; // all components which include the given one
    std::map<std::string, std::vector<deep_dependency>> dependencies; // all dependencies and its parents
    std::map<std::string, event> events; // all dependencies and its parents

    // used by some sections
    int scope_start{ 0 };
    int scope_count{ 0 };

    // some general helpers
    Parser parser;

    auto create_container = [](const std::string & word) { containers.push_back(word); containers.back().file = parser.get_current_file().name; };
    auto create_component = [](const std::string & word) { components.push_back(word); create_container(word); components.back().container_index = int(containers.size()) - 1; containers.back().component = true; };
    auto set_as_manager = [](const std::string& word) { containers.back().manager = true; };
    auto set_manager_extension = [](const std::string& word) { containers.back().manager_extension = word; };
    auto add_dependency_component = [](const std::string & word) { components.back().dependencies.push_back(word); };
    auto expect_anything = [](const std::string & word) -> bool { return true; };
    auto do_nothing = [](const std::string & word) {};
    auto is_equal = [](const std::string& compare) { return [compare](const std::string& word) -> bool { return word == compare; }; };
    auto is_name = [](const std::string & word) -> bool { return (word[0] > 'a' && word[0] < 'z') || (word[0] > 'A' && word[0] < 'Z'); };
    auto is_number = [](const std::string & word) -> bool { return (word[0] > '0' && word[0] < '9'); };
    auto is_string = [](const std::string & word) -> bool { return word[0] == '"'; };
    auto create_resource = [](const std::string& word) { resources.push_back(word); create_container(word); resources.back().container_index = int(containers.size()) - 1; containers.back().resource = true; };
    auto add_extension = [](const std::string & word) { resources.back().extensions.push_back(word); };
    auto set_owner_knowledge = [](const std::string & word) { components.back().owner_knowledge = true; };
    auto create_property = [](const std::string & word) { containers.back().properties.emplace_back(); };
    auto save_property_name = [](const std::string & word) { containers.back().properties.back().name = word; };
    auto set_property_as_function = [](const std::string & word) { containers.back().properties.back().is_function = true; };
    auto add_argument = [](const std::string & word) { containers.back().properties.back().arguments.emplace_back(word); };
    auto add_numeric_argument = [](const std::string & word) { containers.back().properties.back().numeric_arguments.emplace_back(word); };
    auto add_string_argument = [](const std::string & word) { containers.back().properties.back().string_arguments.emplace_back(word); };
    auto add_type_argument = [](const std::string& word) { containers.back().properties.back().type_arguments.emplace_back(word); };
    auto add_default_property_value = [](const std::string & word) { containers.back().properties.back().default_value += word; };
    auto create_enum = [](const std::string & word) { 
        enums.push_back(word); enums.back().file = parser.get_current_file().name; };
    auto remove_enum = [](const std::string & word) { enums.pop_back(); };
    auto add_enum_name = [](const std::string & word) { enums.back().elements.emplace_back(word); };
    auto set_starting_scope = [](const std::string & word) { scope_start = scope_count; };
    auto check_exit_scope = [](const std::string & word) -> bool { return word[0] == '}' && scope_start > scope_count; };

    auto comment_line_token_packer = [](const std::string & text, size_t index, bool& add) ->size_t 
    { 
        if (text.size() - index < 4) return 0;
        if (text.substr(index, 2) == "/*")
        {
            size_t end{ index + 2 };
            add = false;
            const auto text_size = text.size() - 1;

            for (; text.substr(end, 2) != "*/" && end < text_size; end++);
            return end - index + 1;
        }
        else return 0;
    };
    auto comment_block_token_packer = [](const std::string & text, size_t index, bool& add) ->size_t 
    { 
        if (text.size() - index < 3) return 0;
        if (text.substr(index, 2) == "//")
        {
            size_t end{ index + 2 };
            add = false;
            const auto text_size = text.size() - 1;

            for (; text[end] != '\n' && end < text_size; end++);
            return end - index + 1;
        }
        else return 0;
    };
    auto string_tocken_packer = [](const std::string & text, size_t index, bool& add) ->size_t 
    { 
        if (text.size() - index < 2) return 0;
        if (text[index] == '\"')
        {
            size_t end{ index + 1 };
            add = true;

            for (; !(text[end] == '\"' && text[end - 1] != '\\');  end++);
            return end - index + 1;
        }
        else return 0;
    };
    auto custom_tocken_checker = [](char character)
    {
        if (character == '{') scope_count++;
        else if (character == '}') scope_count--;
    };

    void parse_files();
    void generate_components_data();
    void compute_components_data();
    void generate_resources_data();
    void generate_enums_data();
    void generate_containers_data();
    void generate_events_data();
    void create_data_folders();
    void join_all_cpp_files();
    void write_to_files();

    write_file& create_write_file(const std::string & pathname);

    const char* header_comment
    {
        " //------------------------------------------------------------------------------  \n"
        " // <auto-generated>                                                               \n"
        " //    This code was generated from a parser.                                      \n"
        " //                                                                                \n"
        " //    Manual changes to this file will be overwritten if the code is regenerated. \n"
        " // </auto-generated>                                                              \n"
        " //------------------------------------------------------------------------------  \n"
    };
}

int main()
{
    parse_files();
    //create_data_folders();
    //compute_components_data();
    //generate_components_data();
    //generate_resources_data();
    //generate_enums_data();
    //generate_containers_data();
    //generate_events_data();
    //join_all_cpp_files();
    //write_to_files();
}

namespace
{
    void parse_files()
    {
        // -------------------------------------------- parser settings --------------------------------------------
        parser.ignore_files = {};
        parser.ignore_folders = {};
        parser.read_extensions = { ".h", ".cpp" };
        parser.directories_to_read = { "tests" };
        parser.remove_tokens = { ' ' , '\t', '\n' };
        parser.insert_tokens = { ':', ',', ';', '{', '}', '(', ')', '<', '>', '*', '&', '=' };
        parser.token_packers = { comment_line_token_packer, comment_block_token_packer, string_tocken_packer };
        parser.custom_tocken_checker = custom_tocken_checker;

        //// ----------- property segment --------------------------------------------
        //compexe_node(ai, equal_string("{"), set_starting_scope);
        //compexe_node(aj, equal_string("property"), create_property);
        //compare_node(ak, check_exit_scope);
        //compexe_node(al, equal_string("with"), create_property);
        //execute_node(am, expect_anything, save_property_name);
        //compare_node(an, equal_string(";"));
        //compare_node(ap, equal_string("{"));
        //compexe_node(aae, equal_string("("), set_property_as_function);
        //compare_node(aaf, equal_string(";"));
        //execute_node(aag, expect_anything, do_nothing);
        //execute_node(aq, is_name, add_argument);
        //compare_node(ar, equal_string(","));
        //compexe_node(as, is_number, add_numeric_argument);
        //compexe_node(aad, is_name, add_type_argument);
        //execute_node(at, is_string, add_string_argument);
        //execute_node(au, expect_anything, do_nothing);
        //compare_node(av, equal_string("="));
        //execute_node(aw, equal_string("("), do_nothing);
        //execute_node(ax, equal_string(";"), do_nothing);
        //execute_node(az, equal_string(")"), do_nothing);
        //execute_node(aaa, equal_string("property"), do_nothing);
        //compare_node(aab, equal_string("}"));
        //execute_node(aac, expect_anything, add_default_property_value);
        //ai.true_node = &al;
        //aj.false_node = &ak;
        //aj.true_node = &am;
        //ak.false_node = &au;
        //au.next_node = &al;
        //al.false_node = &aj;
        //al.true_node = &aw;
        //aw.next_node = &aq;
        //am.next_node = &an;
        //an.false_node = &ap;
        //an.true_node = &al;
        //ap.false_node = &aae;
        //ap.true_node = &aab;
        //aae.false_node = &am;
        //aae.true_node = &aaf;
        //aaf.true_node = &al;
        //aaf.false_node = &aag;
        //aag.next_node = &aaf;
        //aab.false_node = &aac;
        //aab.true_node = &ax;
        //aac.next_node = &aab;
        //ax.next_node = &al;
        //aq.next_node = &ar;
        //ar.true_node = &aq;
        //ar.false_node = &av;
        //av.false_node = &az;
        //az.next_node = &aaa;
        //av.true_node = &as;
        //as.false_node = &aad;
        //as.true_node = &ar;
        //aad.false_node = &at;
        //aad.true_node = &ar;
        //at.next_node = &ar;
        //aaa.next_node = &am;
        //
        //// -------------------------------------------- container stream flow --------------------------------------------
        //flow container_flow;
        //compare_node(ca, equal_string("container"));
        //execute_node(cb, is_name, create_container);
        //compexe_node(cc, equal_string("manager"), set_as_manager);
        //compare_node(cd, equal_string("extensions"));
        //execute_node(ce, equal_string("("), do_nothing);
        //execute_node(cf, is_name, set_manager_extension);
        //execute_node(cg, equal_string(")"), do_nothing);
        //container_flow.start = &ca;
        //ca.true_node = &cb;
        //cb.next_node = &cc;
        //cc.false_node = &ai;
        //cc.true_node = &cd;
        //cd.false_node = &ai;
        //cd.true_node = &ce;
        //ce.next_node = &cf;
        //cf.next_node = &cg;
        //cg.next_node = &ai;
        //
        //// -------------------------------------------- component stream flow --------------------------------------------
        //flow component_flow;
        //compare_node(da, equal_string("component"));
        //execute_node(db, is_name, create_component);
        //compare_node(dc, equal_string("with"));
        //execute_node(dd, equal_string("("), do_nothing);
        //execute_node(de, is_name, add_dependency_component);
        //compare_node(df, equal_string(","));
        //execute_node(dg, equal_string(")"), do_nothing);
        //compexe_node(dh, equal_string("owner_knowledge"), set_owner_knowledge);
        //component_flow.start = &da;
        //da.true_node = &db;
        //db.next_node = &dc;
        //dc.true_node = &dd;
        //dc.false_node = &dh;
        //dd.next_node = &de;
        //de.next_node = &df;
        //df.true_node = &de;
        //df.false_node = &dg;
        //dg.next_node = &dh;
        //dh.true_node = dh.false_node = &ai;
        //
        //// -------------------------------------------- resource stream flow --------------------------------------------
        //flow resource_flow;
        //compare_node(ea, equal_string("resource"));
        //execute_node(eb, is_name, create_resource);
        //execute_node(ec, equal_string("extensions"), do_nothing);
        //execute_node(ed, equal_string("("), do_nothing);
        //execute_node(ee, is_name, add_extension);
        //compare_node(ef, equal_string(","));
        //execute_node(eg, equal_string(")"), do_nothing);
        //resource_flow.start = &ea;
        //ea.true_node = &eb;
        //eb.next_node = &ec;
        //ec.next_node = &ed;
        //ed.next_node = &ee;
        //ee.next_node = &ef;
        //ef.true_node = &ee;
        //ef.false_node = &eg;
        //eg.next_node = &ai;
        //
        //// ------------------------------------------- enum stream flow ----------------------------------------------
        //flow enum_flow;
        //compare_node(ba, equal_string("enum"));
        //execute_node(bb, is_name, create_enum);
        //compexe_node(bc, equal_string(";"), remove_enum);
        //execute_node(bd, equal_string("{"), do_nothing);
        //execute_node(bf, is_name, add_enum_name);
        //compare_node(bg, equal_string("="));
        //compare_node(bh, equal_string(","));
        //execute_node(bi, equal_string("}"), do_nothing);
        //execute_node(bj, expect_anything, do_nothing);
        //enum_flow.start = &ba;
        //ba.true_node = &bb;
        //bb.next_node = &bc;
        //bc.false_node = &bd;
        //bd.next_node = &bf;
        //bf.next_node = &bg;
        //bg.false_node = &bh;
        //bg.true_node = &bj;
        //bj.next_node = &bh;
        //bh.true_node = &bf;
        //bh.false_node = &bi;
        //
        //// stream flows
        //parser.stream_flows.push_back(component_flow);
        //parser.stream_flows.push_back(resource_flow);
        //parser.stream_flows.push_back(container_flow);
        //parser.stream_flows.push_back(enum_flow);
        //

        
        // ------------------------------------------- enum flow ----------------------------------------------
        node_ptr A, B, C, D, E, F, G, H, I, null;
        compare_node(A, is_equal("enum"), B, null);
        execute_node(B, is_name, create_enum, C);
        compexe_node(C, is_equal(";"), remove_enum, null, D);
        execute_node(D, is_equal("{"), do_nothing, E);
        execute_node(E, is_name, add_enum_name, F);
        compare_node(F, is_equal("="), I, G);
        compare_node(G, is_equal(","), E, H);
        execute_node(H, is_equal("}"), do_nothing, G);
        execute_node(I, expect_anything, do_nothing, null);
        parser.stream_flows.emplace_back(A);

        auto count = B.use_count();

        // execute parser
        parser.parse();
    }

    void compute_components_data()
    {
        enums.push_back(std::string("component_type")); 
        enums.back().file = "component_type.h";

        for (auto & component : components)
        {
            auto& current_dependents = dependants[component.name];
            add_enum_name("component_type_" + component.name);

            // get all the components which have a dependency with this one
            std::vector<::component> components_to_check;
            components_to_check.push_back(component);

            while (!components_to_check.empty())
            {
                auto component_name = components_to_check.begin()->name;
                components_to_check.erase(components_to_check.begin());

                // check all the components dependencies
                for (auto& component : components)
                {
                    for (auto& dependency : component.dependencies)
                    {
                        // if this component is one of its dependencies
                        if (dependency == component_name)
                        {
                            bool added{ false };

                            // check it is not already added
                            for (auto& dependent : current_dependents)
                                if (dependent.name == component.name)
                                    added = true;

                            if (!added)
                            {
                                components_to_check.push_back(component);
                                current_dependents.push_back(component);
                                break;
                            }
                        }
                    }
                }
            }

            auto& deep_dependencies = dependencies[component.name];
            direct_dependents[component.name];

            // compute deep dependencies
            for (auto& dependency_name : component.dependencies)
            {
                direct_dependents[dependency_name].push_back(component.name);

                for (auto& component : components)
                {
                    if (component.name == dependency_name)
                    {
                        deep_dependencies.emplace_back();
                        deep_dependencies.back().component = &component;
                        break;
                    }
                }
            }

            for (auto i = 0; i < deep_dependencies.size(); i++)
            {
                auto& current_dependency = deep_dependencies[i];

                // if the dependency has a dependency that is not already added				
                // get dependency
                for (auto& component : components)
                {
                    if (component.name == current_dependency.component->name)
                    {
                        // check dependencies of the component
                        for (auto& dependency : component.dependencies)
                        {
                            // check if dependency is not added
                            bool added{ false };
                            for (auto& added_dependency : deep_dependencies)
                            {
                                if (dependency == added_dependency.component->name)
                                {
                                    added = true;
                                    break;
                                }
                            }

                            if (!added)
                            {
                                for (auto& component : components)
                                {
                                    if (component.name == dependency)
                                    {
                                        deep_dependency new_dependency{component};
                                        new_dependency.parents.insert(new_dependency.parents.end(), current_dependency.parents.begin(),
                                            current_dependency.parents.end());
                                        new_dependency.parents.push_back(current_dependency.component->name);
                                        deep_dependencies.push_back(new_dependency);
                                    }
                                }
                            }
                        }
                        break;
                    }
                }
            }
        }

        for (auto& container : containers)
        {
            for (auto& property : container.properties)
            {
                auto numeric_arguments_index = 0;
                auto string_arguments_index = 0;
                auto type_arguments_index = 0;

                if(property.is_function)
                    property.attribute_not_serializable = true;

                for (auto& attribute : property.arguments)
                {
                    if (attribute == "attribute_not_visible")
                        property.attribute_not_visible = true;

                    else if (attribute == "attribute_not_resizeable")
                        property.attribute_not_resizeable = true;

                    else if (attribute == "attribute_not_serializable")
                        property.attribute_not_serializable = true;

                    else if (attribute == "attribute_event")
                    {
                        const auto& event_name = property.type_arguments[type_arguments_index++];
                        events[event_name].component_funtions[container.name].push_back(property.name);
                        property.is_event = true;
                    }  

                }
            }
        }
    }

    void generate_components_data()
    {
        // -------------------------------------------- component.h --------------------------------------------
        auto & component_h = create_write_file("../poly/data/components/component.h");

        component_h << header_comment <<
        "#pragma once							\n"
        "#include \"smart_vector.h\"			\n"
        "#include \"component_type.h\"		    \n"
        "#include \"utils.h\"		            \n"
        "#define component struct				\n"
        "#define with(...) 						\n"
        "#define property						\n"
        "#define owner_knowledge	            \n"
        "										\n"
        "enum attribute						    \n"
        "{										\n"
        "    attribute_not_visible,             \n"
        "    attribute_not_resizeable,          \n"
        "    attribute_not_serializable,        \n"
        "    attribute_event,                   \n"
        "};										\n\n";

        for (auto& component : components)
        {
            component_h << "component " << component.name << ";\n";
        }
        component_h << "\n";

        for (auto & component : components)
        {
            auto& deep_dependencies = dependencies[component.name];

            component_h << "#define " << component.name << "_data" << "  " << "\\\n";
            component_h << "public: \\\n";
            component_h << "friend bool show_in_editor(" << component.name << " & type);\\\n";

            for (auto& dependent : dependants[component.name])
                component_h << "friend bool show_in_editor(" << dependent.name << " & type);\\\n";

            component_h << "friend void copy(" << component.name << " & to, const Json::Value & from);\\\n";
            component_h << "friend void save(const " << component.name << " & value, Json::Value & json);\\\n";
            component_h << "friend void load(" << component.name << " & value, const Json::Value & json);\\\n";

            component_h << "static " << component.name << "& get();\\\n";
            component_h << "static handle<" << component.name << "> add_to_gameobject(gameobject & gameobject); \\\n";
            component_h << "static void remove_from_gameobject(gameobject & gameobject); \\\n";
            component_h << "static " << component.name << " & get(const handle<" << component.name << "> &);\\\n";
            component_h << "static inline component_type type(){ return component_type_" << component.name << "; }\\\n";
            component_h << "inline handle<" << component.name << "> get_handle() const { return " << component.name << "::array.get(this); }\\\n";
            component_h << "template<typename t> inline t* get() { return nullptr; }\\\n";

            component_h << "template<> inline " << component.name << "* get<" << component.name << ">() { return this; }\\\n";
            for (auto& dependency : deep_dependencies)
            {
                component_h << "template<> inline " << dependency.component->name << "* get<"
                    << dependency.component->name << ">() { return &";
                    
                    for (auto& parent : dependency.parents)
                    {
                        component_h << "get_" << parent << "().";
                    }
                    
                    component_h << "get_" << dependency.component->name << "(); }\\\n";
            }

            for (auto & dependency : component.dependencies)
            {
                component_h << dependency << " & " << "get_" << dependency << "() const;\\\n";
            }

            for (auto& dependency : component.dependencies)
            {
                component_h << "const handle<" << dependency << "> & " << "get_" << dependency << "_handle() const;\\\n";
            }

            if (component.owner_knowledge)
                component_h << "gameobject& get_owner() const;\\\n";

            component_h << "static smart_vector<" << component.name << "> array;\\\n";

            component_h << "private: \\\n";
            if(component.owner_knowledge)
                component_h << "handle<gameobject> owner;\\\n";

            for (auto & dependency : component.dependencies)
            {
                component_h << "handle<" << dependency << "> " << dependency << "_handle; \\\n";
            }

            for (auto & dependent : dependants[component.name])
            {
                component_h << "friend struct " << dependent.name << ";\\\n";
            }

            component_h << '\n';
        }

        // -------------------------------------------- components.cpp --------------------------------------------
        for (auto & component : components)
        {
            auto& component_cpp = create_write_file("../poly/data/components/component_" + component.name + ".cpp");
            auto& deep_dependencies = dependencies[component.name];

            component_cpp << header_comment;
            component_cpp << "#include \"" << containers[component.container_index].file << "\"\n";
            component_cpp << "#include \"serialization.h\"\n";
            component_cpp << "#include \"show_in_editor.h\"\n";
            component_cpp << "#include \"component_type.h\"\n";

            for (auto & dependency : component.dependencies)
            {
                for (auto & component : components)
                {
                    if (component.name == dependency)
                    {
                        component_cpp << "#include \"" << containers[component.container_index].file << "\"\n\n";
                        break;
                    }
                }
            }
            component_cpp << "\n";

            if (component.owner_knowledge)
                component_cpp << "gameobject & " << component.name << "::get_owner() const { return gameobject::get(owner); }\n";

            component_cpp << "smart_vector<" << component.name << "> " << component.name << "::array{}; \n";	
            component_cpp << component.name << "& " << component.name << "::get(){ return " << component.name << "::array.front(); };\n";
            component_cpp << component.name << " & "<< component.name << "::get(const handle<" << component.name
                << "> & handle){ return " << component.name << "::array.get(handle); }\n";

            for (auto & dependency : component.dependencies)
            {
                component_cpp << dependency << " & " << component.name << "::get_" << dependency << "() const ";
                component_cpp << "{ return " << dependency << "::array.get(" << dependency << "_handle" << "); }\n";
            }

            for (auto& dependency : component.dependencies)
            {
                component_cpp << "const handle<" << dependency << "> & " << component.name << "::get_" << dependency << "_handle() const ";
                component_cpp << "{ return " << dependency << "_handle" << "; }\n";
            }
            component_cpp << '\n';


            component_cpp << "handle<" << component.name << "> " << component.name << "::add_to_gameobject(gameobject & gameobject) \n";
            component_cpp << "{\n";
            component_cpp << "    for(auto & component_id : gameobject.components)\n";
            component_cpp << "        if(component_id.type == component_type_" << component.name << ")\n";
            component_cpp << "            return static_cast<::handle<" << component.name << ">>(component_id.handle);\n\n";
            component_cpp << "    const auto & handle" << " = " << component.name << "::array.push(std::move(" << component.name << "{}));\n";
            component_cpp << "    auto & new_component = " << component.name << "::get(handle);\n";
            if(component.owner_knowledge)
                component_cpp << "    new_component.owner = gameobject.get_handle();\n";
            component_cpp << "    gameobject.components.push_back(component_id{ component_type_" << component.name << ", static_cast<::handle<void>>(handle) });\n";
            for (auto & dependency : component.dependencies)
            {
                component_cpp << "    new_component." << dependency << "_handle = " << dependency << "::add_to_gameobject(gameobject);\n";
            }
            component_cpp << "    return handle;\n";
            component_cpp << "}\n\n";


            component_cpp << "void " << component.name << "::remove_from_gameobject(gameobject & gameobject)\n";
            component_cpp << "{\n";
            component_cpp << "    auto& array = gameobject.components;\n";
            component_cpp << "    for(auto it = array.begin(); it != array.end(); it++)\n";
            component_cpp << "    {\n";
            component_cpp << "        auto& component_id = *it;\n";
            component_cpp << "        if(component_id.type == component_type_" << component.name << ")\n";
            component_cpp << "        {\n";
            component_cpp << "            destroy_delayed(component_id);\n";
            component_cpp << "            for(auto & dependant :" << component.name << "_dependants)\n";
            component_cpp << "                remove_component(gameobject, dependant);\n";
            component_cpp << "            array.erase(it);\n";
            component_cpp << "            break;\n";
            component_cpp << "        }\n";
            component_cpp << "    }\n";
            component_cpp << "}\n";


            
            component_cpp << "bool show_in_editor(" << component.name << "& type)\n";
            component_cpp << "{\n";
            component_cpp << "    auto focus = false;\n";
            for (auto& property : containers[component.container_index].properties)
            {
                if (!property.attribute_not_visible && !property.is_event)
                {
                    if (property.is_function)
                        component_cpp << "    focus |= type." << property.name << "(); \n";
                    else
                    {
                        component_cpp << "    focus |= show_in_editor(\"" << property.name << "##" << component.name << "\", type.get_handle(), type." << property.name << ", true";

                        if (property.attribute_not_resizeable)
                            component_cpp << ", false";

                        component_cpp << ");\n";
                    }
                }
            }
            component_cpp << "    return focus;\n";
            component_cpp << "}\n\n";


            component_cpp << "void copy(" << component.name << " & to, const Json::Value & from)\n";
            component_cpp << "{\n";
            for (auto& property : containers[component.container_index].properties)
            {
                if (property.attribute_not_serializable)
                    continue;

                component_cpp << "    load(to." << property.name << ", from[\"" << property.name << "\"]";
                if (!property.default_value.empty())  component_cpp << ", decltype(to." << property.name << "){ " << property.default_value << " }";
                component_cpp << ");\n";
            }            
            component_cpp << "}\n\n";


            component_cpp << "void load(" << component.name << " & value, const Json::Value & json)\n";
            component_cpp << "{\n";
            for (auto& property : containers[component.container_index].properties)
            {
                if (property.attribute_not_serializable)
                    continue;

                component_cpp << "    load(value." << property.name << ", json[\"" << property.name << "\"]";                
                if (!property.default_value.empty())  component_cpp << ", decltype(value." << property.name << "){ " << property.default_value << " }";
                component_cpp << ");\n";
            }
            for (auto& dependency : component.dependencies)
            {
                component_cpp << "    load(value." << dependency << "_handle, json[\"" << dependency << "_handle\"]);\n";
            }
            if (component.owner_knowledge) component_cpp << "    load(value.owner, json[\"owner\"]);\n";
            component_cpp << "}\n\n";

            component_cpp << "void save(const " << component.name << " & value, Json::Value & json)\n";
            component_cpp << "{\n";
            for (auto& property : containers[component.container_index].properties)
            {
                if (property.attribute_not_serializable)
                    continue;

                component_cpp << "    save(value." << property.name << ", json[\"" << property.name << "\"]" << ");\n";
            }
            for (auto& dependency : component.dependencies)
            {
                component_cpp << "    save(value." << dependency << "_handle, json[\"" << dependency << "_handle\"]);\n";
            }
            if (component.owner_knowledge) component_cpp << "    save(value.owner, json[\"owner\"]);\n";
            component_cpp << "}\n";

        }
        // -------------------------------------------- component_type.h --------------------------------------------
        auto& component_type_h = create_write_file("../poly/data/components/component_type.h");
        component_type_h << header_comment <<
            "#pragma once\n"
            "#include \"smart_vector.h\"\n";

        component_type_h << "\n";
        component_type_h << "enum component_type\n{\n";
        for (auto i = 0; i < components.size(); i++)
        {
            auto& component = components[i];

            if (i == components.size() - 1)
                component_type_h << "    component_type_" << component.name << '\n';
            else
                component_type_h << "    component_type_" << component.name << ",\n";
        }
        component_type_h << "};\n\n";


        component_type_h << "struct component_id\n";
        component_type_h << "{\n";
        component_type_h << "    component_type type;\n";
        component_type_h << "    handle<void> handle;\n";
        component_type_h << "};\n\n";


        for (auto& component : components)
        {
            const auto& dependans_array = dependants[component.name];
            const auto dependans_count = dependans_array.size();

            component_type_h << "constexpr std::array<component_type, " << std::to_string(dependans_count) << "> " << component.name << "_dependants\n";
            component_type_h << "{\n";

            for (auto& dependant : dependants[component.name])
            {
                component_type_h << "    component_type_" << dependant.name << ",\n";
            }
            component_type_h << "};\n\n";
        }


        // -------------------------------------------- component_data.h --------------------------------------------
        auto& component_data_h = create_write_file("../poly/data/components/component_data.h");
        component_data_h << header_comment <<
            "#pragma once                       \n"
            "#include \"all_components.h\"      \n"
            "#include \"smart_vector.h\"        \n"
            "#include \"component_type.h\"      \n\n";

        component_data_h <<
            "handle<void> add_component(gameobject & gameobject, component_type type);\n"
            "void remove_component(gameobject & gameobject, component_type type);\n"
            "void restore_component(component_id id, const Json::Value & json);\n"
            "void clear_all_components();							\n"
            "void save_all_components(Json::Value &);               \n"
            "void load_all_components(const Json::Value &);         \n"
            "bool show_in_editor(const component_id & id);          \n"
            "void save_gameobject(const gameobject & from, Json::Value & to);\n"
            "void copy_component(const component_id & to, const Json::Value & from);\n"
            "void save_component(const component_id & id, Json::Value & json);\n"
            "void load_component(const component_id & id, const Json::Value & json);\n"
            "void destroy_component(const component_id & id);                 \n"
            "                                                    \n"
            "template<typename t>                                \n"
            "t* get_component(const component_id & id)                 \n"
            "{           					                     \n";

        component_data_h << 
            "    switch(id.type)\n"
            "    {\n";

        for (auto& component : components)
            component_data_h << "        case component_type_" << component.name << ": return " << component.name << "::get(static_cast<handle<"<< component.name << ">>(id.handle)).get<t>(); break;\n";

        component_data_h << "        default: return nullptr; break;\n"
            "    }\n"
            "}\n\n";

        // -------------------------------------------- component_data.cpp --------------------------------------------
        auto& component_data_cpp = create_write_file("../poly/data/components/component_data.cpp");
        component_data_cpp << header_comment <<
            "#include \"component_data.h\"                      \n"
            "#include \"all_containers.h\"                      \n"
            "#include \"utils.h\"                               \n"
            "#include \"serialization.h\"                       \n"
            "                                                   \n"
            "handle<void> add_component(gameobject & gameobject, component_type type)\n"
            "{                                                  \n"
            "    switch(type)\n"
            "    {\n";

        for (auto& component : components)
            component_data_cpp << "        case component_type_" << component.name << ": return static_cast<handle<void>>(gameobject.add_component<" << component.name << ">().get_handle()); break;\n";

        component_data_cpp << "        default: error(\"using non existing component type\"); return handle<void>{};\n"
            "    };\n"
            "} \n\n";


        component_data_cpp << "void remove_component(gameobject & gameobject, component_type type)\n"
            "{\n"
            "    switch(type)\n"
            "    {\n";

        for (auto& component : components)
            component_data_cpp << "        case component_type_" << component.name << ": gameobject.remove_component<" << component.name << ">(); break;\n";

        component_data_cpp << "        default: error(\"using non existing component type\");\n"
            "    };\n"
            "} \n\n";


        component_data_cpp << "void restore_component(component_id id, const Json::Value & json)\n"
            "{\n"
            "    switch(id.type)\n"
            "    {\n";

        for (auto& component : components)
            component_data_cpp << "        case component_type_" << component.name << ": " << component.name << "::array.push(std::move(" << component.name << "{}), static_cast<handle<" << component.name << ">>(id.handle)); load(" << component.name << "::get(static_cast<handle<" << component.name << ">>(id.handle)), json); break;\n";

        component_data_cpp << "        default: error(\"using non existing component type\");\n"
            "    };\n"
            "} \n\n";


        component_data_cpp <<  "void clear_all_components() \n"
            << "{\n";
        for (auto& component : components)
            component_data_cpp << "    " << component.name << "::array.clear();\n";
        component_data_cpp << "}\n\n";


        component_data_cpp << "\n" << "void save_all_components(Json::Value& json)\n"
            << "{\n";     
        for (auto& component : components)
            component_data_cpp << "    save<" << component.name << ">(" << component.name << "::array, json[\"" << component.name << "\"]);\n";
        component_data_cpp << "}\n\n";


        component_data_cpp << "void load_all_components(const Json::Value & json)\n"
            << "{\n";
        for (auto& component : components)
            component_data_cpp << "    load<" << component.name << ">(" << component.name << "::array, json[\"" << component.name << "\"]);\n";      
        component_data_cpp << "}\n\n";


        component_data_cpp << 
            "bool show_in_editor(const component_id & id)\n"
            "{\n"
            "    switch(id.type)\n"
            "    {\n";

        for (auto& component : components)
        {
            component_data_cpp << "        case component_type_" << component.name << ": return show_in_editor(" << component.name << "::get(static_cast<handle<" << component.name << ">>(id.handle))); break;\n";
        }
        component_data_cpp << "        default: error(\"using non existing component type\"); return false;\n"
            "    };\n"
            "}\n\n";

        component_data_cpp <<
            "void save_gameobject(const gameobject & from, Json::Value & to)\n"
            "{\n"
            "    save(from.get_handle(), to[\"handle\"]);\n"
            "    save(from, to);\n"
            "    for(auto & component_id : from.components)\n"
            "       save_component(component_id, to[get_enum_name(component_id.type).substr(sizeof(\"component_type\")).c_str()]);\n"
            "}\n\n";

      
         component_data_cpp <<
            "void copy_component(const component_id & to, const Json::Value & from)\n"
            "{\n"
            "    switch(to.type)\n"
            "    {\n";

        for (auto& component : components)
        {
            component_data_cpp << "        case component_type_" << component.name << ": copy(" << component.name << "::get(static_cast<handle<" << component.name << ">>(to.handle)), from); break;\n";
        }
        component_data_cpp << "        default: error(\"using non existing component type\");\n"
            "    };\n"
            "}\n\n";


          component_data_cpp <<
            "void save_component(const component_id & id, Json::Value & json)\n"
            "{\n"
            "    switch(id.type)\n"
            "    {\n";

        for (auto& component : components)
        {
            component_data_cpp << "        case component_type_" << component.name << ": save(" << component.name << "::get(static_cast<handle<" << component.name << ">>(id.handle)), json); break;\n";
        }
        component_data_cpp << "        default: error(\"using non existing component type\");\n"
            "    };\n"
            "}\n\n";


        component_data_cpp <<
            "void load_component(const component_id & id, const Json::Value & json)\n"
            "{\n"
            "    switch(id.type)\n"
            "    {\n";

        for (auto& component : components)
        {
            component_data_cpp << "        case component_type_" << component.name << ": load(" << component.name << "::get(static_cast<handle<" << component.name << ">>(id.handle)), json); break;\n";
        }
        component_data_cpp << "        default: error(\"using non existing component type\");\n"
            "    };\n"
            "}\n\n";


        component_data_cpp <<
            "void destroy_component(const component_id & id)\n"
            "{\n"
            "    switch(id.type)\n"
            "    {\n";

        for (auto& component : components)
        {
            component_data_cpp << "        case component_type_" << component.name << ": " << component.name << "::array.remove(static_cast<handle<" << component.name << ">>(id.handle)); break;\n";
        }
        component_data_cpp << "        default: error(\"using non existing component type\");\n"
            "    };\n"
            "}\n\n";

        // -------------------------------------------- all_components.h --------------------------------------------
        auto& all_components_h = create_write_file("../poly/data/components/all_components.h");
        all_components_h << header_comment << "#pragma once\n";

        for (auto& component : components)
            all_components_h << "#include \"" << containers[component.container_index].file << "\"\n";
    }

    void generate_resources_data()
    {
        // -------------------------------------------- resource.h --------------------------------------------
        // overwrite or create resource.h
        auto & resource_h = create_write_file("../poly/data/resources/resource.h");

        resource_h << header_comment <<
            "#pragma once																			            \n"
            "#include \"component.h\"															                    \n"
            "#include \"resource_info.h\"																	    \n"
            "#include \"resource_handle.h\"																	    \n"
            "#include \"utils.h\"                                                                               \n"
            "#define resource struct																            \n"
            "#define extensions(...) 																            \n"
            "#define custom_save      													                        \n"
            "																						            \n";
            
        for (auto& resource : resources)
            resource_h << "resource " << resource.name << ";\n";
        resource_h << "\n";

        resource_h << "void create_resource_info(const std::string & path, const std::string & name, bool create_resource = false);\n";
        resource_h << "void clear_resource_info();\n\n";
        for (auto & resource : resources)
        {
            const auto& container = containers[resource.container_index];
            resource_h << "#define " << resource.name << "_data" << "  " << "\\\n";
            resource_h << "public: \\\n";
            resource_h << "static inline " << resource.name << "& get(const ::handle<" << resource.name << "> & handle){ return " << resource.name << "::array.get(handle); }\\\n";
            resource_h << "static inline resource_handle<" << resource.name << "> get(const std::string & name){ return resource_info<" << resource.name << ">::get(name); }\\\n";
            resource_h << "static " << "smart_vector<" << resource.name << "> array;\\\n";
            resource_h << resource.name << "::" << resource.name << "() = default;\\\n";
            resource_h << "private: \\\n";

            if (!container.properties.empty())
                resource_h << resource.name << "(const resource_info<" << resource.name << "> & info){ load(*this, load_json(info.path, info.name)); }\\\n";
            else
                resource_h << resource.name << "(const resource_info<" << resource.name << "> & info){ load(info); }\\\n";


            resource_h << "static inline ::handle<" << resource.name << "> create(const resource_info<" << resource.name << ">& info){ return " << resource.name << "::array.push(std::move(" << resource.name << "{info})); } \\\n";
            resource_h << "friend struct resource_info<" << resource.name << ">;\\\n"; 

            if (!container.properties.empty())
            {
                resource_h << "template<typename u> friend bool show_in_editor(const std::string& name, const handle<u>& handle, " << container.name << " & type, bool command);\\\n";
                resource_h << "friend void save(const " << container.name << " & value, Json::Value & json);\\\n";
                resource_h << "friend void load(" << container.name << " & value, const Json::Value & json);\\\n";
            }

            resource_h << "\n";		
        }     
        resource_h << '\n';

        for (auto& container : containers)
        {
            if (!container.resource) continue;

            resource_h << "template<typename u>\n";
            resource_h << "bool show_in_editor(const std::string & name, const handle<u>& handle, " << container.name << "& type, bool command)\n";
            resource_h << "{\n";
            resource_h << "    auto focus = false;\n";
            resource_h << "    ImGui::PushStyleColor(ImGuiCol_Header, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled) - ImVec4(0.f, 0.f, 0.f, 0.5f));\n";
            resource_h << "    if(ImGui::CollapsingHeader(name.c_str()))\n";
            resource_h << "    {\n";
            for (auto& property : container.properties)
            {
                if (!property.attribute_not_visible && !property.is_event)
                {
                    if (property.is_function)
                        resource_h << "        focus |= type." << property.name << "(); \n";
                    else
                    {
                        resource_h << "        focus |= show_in_editor(\"" << property.name << "##" << container.name << "_\" + name, handle, type." << property.name << ", command";

                        if (property.attribute_not_resizeable)
                            resource_h << ", false";

                        resource_h << ");\n";
                    }
                }
            }
            resource_h << "        ImGui::Separator();\n";
            resource_h << "    }\n";
            resource_h << "    ImGui::PopStyleColor();\n";
            resource_h << "    return focus;\n";
            resource_h << "}\n\n";
        }

        // -------------------------------------------- resources.cpp --------------------------------------------

        auto & resource_cpp = create_write_file("../poly/data/resources/resource.cpp");
        resource_cpp << header_comment;
        resource_cpp <<	"#include \"resource.h\"\n";
        resource_cpp << "#include \"serialization.h\"\n";
        resource_cpp << "#include \"show_in_editor.h\"\n";

        for (auto & resource : resources)
            resource_cpp << "#include \"" << containers[resource.container_index].file << "\"\n";

        for (auto& container : containers)
            if (!container.manager_extension.empty())
                resource_cpp << "#include \"" << container.file << "\"\n";

        resource_cpp << '\n';
        resource_cpp << "void create_resource_info(const std::string & path, const std::string & name, bool create_resource)\n";
        resource_cpp << "{\n";
        resource_cpp << "    std::string extension{ PathFindExtension(name.c_str()) + 1 };\n";

        for (auto & resource : resources)
        {
            resource_cpp << "    if(";

            for (auto i = 0; i < resource.extensions.size(); i++)
            {
                auto & extension = resource.extensions[i];
                resource_cpp << "extension == \"" << extension << "\"";

                if (i < resource.extensions.size() - 1)
                {
                    resource_cpp << " || ";
                }
            }

            resource_cpp << "){ resource_info<" << resource.name << ">::create(path, name, create_resource); }\n";
        }

        // loop through all the managers with extensions
        for (auto& container : containers)
        {
            const auto& extension = container.manager_extension;
            if (!extension.empty())
                resource_cpp << "    if(extension == \"" << extension << "\"){ resource_info<" << container.name << ">::create(path, name, create_resource); }\n";
        }
        resource_cpp << "}\n";


        resource_cpp << "void clear_resource_info()\n";
        resource_cpp << "{\n";
        for (auto& resource : resources)
            resource_cpp << "    resource_info<" << resource.name << ">::array.clear();\n";
        // loop through all the managers with extensions
        for (auto& container : containers)
            if (!container.manager_extension.empty())
                resource_cpp << "    resource_info<" << container.name << ">::array.clear();\n";
        resource_cpp << "}\n\n";


        for (auto & resource : resources)
        {
            auto & resource_cpp = create_write_file("../poly/data/resources/resource_" + resource.name + ".cpp");
            resource_cpp << header_comment;
            resource_cpp << "#include \"" << containers[resource.container_index].file << "\"\n";
            resource_cpp << "#include \"serialization.h\"\n\n";

            resource_cpp << "smart_vector<" << resource.name << "> " << resource.name << "::array{}; \n\n";

            const auto& container = containers[resource.container_index];

            resource_cpp << "void load(" << container.name << " & value, const Json::Value & json)\n";
            resource_cpp << "{\n";
            for (auto& property : container.properties)
            {
                if (property.attribute_not_serializable)
                    continue;

                resource_cpp << "    load(value." << property.name << ", json[\"" << property.name << "\"]";
                if (!property.default_value.empty())  resource_cpp << ", decltype(value." << property.name << "){ " << property.default_value << " }";
                resource_cpp << ");\n";
            }
            resource_cpp << "}\n\n";

            resource_cpp << "void save(const " << container.name << " & value, Json::Value & json)\n";
            resource_cpp << "{\n";
            for (auto& property : container.properties)
            {
                if (property.attribute_not_serializable)
                    continue;

                resource_cpp << "    save(value." << property.name << ", json[\"" << property.name << "\"]" << ");\n";
            }
            resource_cpp << "}\n";
        }
    }

    void generate_enums_data()
    {
        // -------------------------------------------- enums.h --------------------------------------------
        // overwrite or create component.h
        auto & enums_h = create_write_file("../poly/data/enums/enums.h");
        enums_h << header_comment << "#pragma once\n"
            "#include \"utils.h\"\n";
         
        for (auto& current_enum : enums)
            enums_h << "#include \""<< current_enum.file << "\"\n";
        enums_h << "\n";

        for (auto& current_enum : enums)
        {
            enums_h << "constexpr std::array<enum_element<" << current_enum.name << ">, " << std::to_string(current_enum.elements.size()) << "> " << current_enum.name << "_data\n{\n";
            const auto enum_size = current_enum.elements.size();

            for (auto i = 0;; i++)
            {
                const auto & element = current_enum.elements[i];

                enums_h << "    enum_element<" << current_enum.name << ">{ \"" << element << "\", " << element;
                
                if (i != enum_size - 1)
                {
                    enums_h << " },\n";
                }
                else
                {
                    enums_h << " }\n}; \n\n";
                    break;
                }
            } 
           
        }
        enums_h << "\n";

        enums_h << "template<typename t> std::vector<enum_element<t>> get_enum_data();\n";
        for (auto& current_enum : enums)
        {
            enums_h << "template<> inline std::vector<enum_element<" << current_enum.name << ">> get_enum_data<" << current_enum.name << ">(){ return std::vector<enum_element<" << current_enum.name << ">>{ " << current_enum.name << "_data.begin(), " << current_enum.name << "_data.end() }; }\n";
        }
        enums_h << "\n";

        enums_h << "template<typename t> t get_enum_value(const std::string & name);\n";
        for (auto& current_enum : enums)
        {
            const auto array_name = current_enum.name + "_data";

            enums_h << "template<> inline " << current_enum.name << " get_enum_value<" << current_enum.name 
                << ">(const std::string & name){ for(auto& e : " << array_name << ") if(e.name == name) return e.value; "
                "warning(\"enum " << current_enum.name << " does not have name \" + name); return " << current_enum.name << "{}; }\n";

        }
        enums_h << "\n";

        for (auto& current_enum : enums)
        {
            const auto array_name = current_enum.name + "_data";
            enums_h << "inline std::string get_enum_name(" << current_enum.name << " value){ for(auto& e : " << array_name << ") if(e.value == value) return e.name; "
                "warning(\"enum " << current_enum.name << " does not have value \" + value); return std::string{}; }\n";
        }
        enums_h << "\n";

        enums_h << "template<typename t> const char * get_enum_name();\n";
        for (auto& current_enum : enums)
            enums_h << "template<> inline const char * get_enum_name<" << current_enum.name << ">(){ return \"" << current_enum.name << "\"; }\n";
        enums_h << "\n";

        // ---------------------------------------- enums_serialization_prototypes.h -------------------------------------
        // overwrite or create enum_serialization.h
        auto& enums_serialization_prototypes_h = create_write_file("../poly/data/enums/enums_serialization_prototypes.h");
        enums_serialization_prototypes_h << header_comment << "#pragma once\n\n";
        for (auto& current_enum : enums)
            enums_serialization_prototypes_h << "inline void save(const " << current_enum.name << " & value, Json::Value & json);\n";
        enums_serialization_prototypes_h << "\n";
        for (auto& current_enum : enums)
            enums_serialization_prototypes_h << "inline void load(" << current_enum.name << " & value, const Json::Value & json);\n";


        // -------------------------------------------- enums_serialization.h --------------------------------------------
        // overwrite or create enum_serialization.h
        auto& enums_serialization_h = create_write_file("../poly/data/enums/enums_serialization.h");
        enums_serialization_h << header_comment << "#pragma once\n";
        enums_serialization_h << "\n";

        for (auto& current_enum : enums)
        {
            enums_serialization_h << "inline void save(const " << current_enum.name 
                << " & value, Json::Value & json){ save(get_enum_name(value), json); }\n";
        }

        enums_serialization_h << "\n";

        for (auto& current_enum : enums)
        {
            enums_serialization_h << "inline void load(" << current_enum.name
                << " & value, const Json::Value & json){ value = get_enum_value<"
                << current_enum.name << ">(json.asString()); }\n";
        }

        enums_serialization_h << "\n";
    }

    void generate_containers_data()
    {
        // --------------------------------------- all_containers.h -----------------------------------------
        auto& all_containers_h = create_write_file("../poly/data/containers/all_containers.h");
        all_containers_h << header_comment << "#pragma once\n";

        for (auto& container : containers)
        {
            if (container.component) continue;
            all_containers_h << "#include \"" << container.file << "\"\n";
        }

        // -------------------------------------------- container.h --------------------------------------------
        auto& container_h = create_write_file("../poly/data/containers/container.h");

        // always needed content
        container_h << header_comment;
        container_h << "#pragma once\n";
        container_h << "#include \"component.h\"\n";
        container_h << "#define container struct\n";
        container_h << "#define manager\n";
        container_h << "#define extensions(...)\n\n";


        for (auto& container : containers)
        {
            if (container.component) continue;

            container_h << "container " << container.name << ";\n";
        }
        container_h << "\n";

        for (auto& container : containers)
        {
            if (!container.free()) continue;

            container_h << "#define " << container.name << "_data" << "  " << "\\\n";
            container_h << "public: \\\n";
            container_h << "template<typename u> friend bool show_in_editor(const std::string& name, const handle<u>& handle, struct " << container.name << " & type, bool command = true);\\\n";
            container_h << "friend void save(const " << container.name << " & value, Json::Value & json);\\\n";
            container_h << "friend void load(" << container.name << " & value, const Json::Value & json);\\\n";

            if (container.manager)
            {
                container_h << "static inline " << container.name << "& get(const handle<" << container.name << ">& handle = handle<" << container.name << ">{}){ static auto instance = " << container.name << "{}; return instance; }\\\n";
                container_h << "inline handle<" << container.name << "> get_handle() const { return handle<" << container.name << ">{}; }\\\n";
                
                if (!container.manager_extension.empty())
                    container_h << "static inline handle<" << container.name << "> create(const resource_info<" << container.name << ">& info){ return handle<" << container.name << ">{}; } \\\n";
            }
            container_h << '\n';
        }
        container_h << '\n';

        for (auto& container : containers)
        { 
            if (!container.free()) continue;

            container_h << "template<typename u>\n";
            container_h << "bool show_in_editor(const std::string & name, const handle<u>& handle, struct " << container.name << "& type, bool command)\n";
            container_h << "{\n";
            container_h << "    auto focus = false;\n";
            container_h << "    ImGui::PushStyleColor(ImGuiCol_Header, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled) - ImVec4(0.f, 0.f, 0.f, 0.5f));\n";
            container_h << "    if(ImGui::CollapsingHeader(name.c_str()))\n";
            container_h << "    {\n";
            for (auto& property : container.properties)
            {
                if (!property.attribute_not_visible && !property.is_event)
                {
                    if (property.is_function)
                        container_h << "        focus |= type." << property.name << "(); \n";
                    else
                    {
                        container_h << "        focus |= show_in_editor(\"" << property.name << "##" << container.name << "_\" + name, handle, type." << property.name << ", command";

                        if (property.attribute_not_resizeable) 
                            container_h << ", false";

                        container_h << ");\n";
                    }
                }
            }
            container_h << "        ImGui::Separator();\n";
            container_h << "    }\n";
            container_h << "    ImGui::PopStyleColor();\n";
            container_h << "    return focus;\n";
            container_h << "}\n\n";
        }

        // -------------------------------------------- containers.cpp --------------------------------------------
        for (auto& container : containers)
        {
            if (!container.free()) continue;

            auto& container_cpp = create_write_file("../poly/data/containers/container_" + container.name + ".cpp");

            container_cpp << header_comment;
            container_cpp << "#include \"" << container.file << "\"\n";
            container_cpp << "#include \"serialization.h\"\n";
            container_cpp << "#include \"show_in_editor.h\"\n\n";


            container_cpp << "void load(" << container.name << " & value, const Json::Value & json)\n";
            container_cpp << "{\n";
            for (auto& property : container.properties)
            {
                if (property.attribute_not_serializable)
                    continue;

                container_cpp << "    load(value." << property.name << ", json[\"" << property.name << "\"]";
                if (!property.default_value.empty())  container_cpp << ", decltype(value." << property.name << "){ " << property.default_value << " }";
                container_cpp << ");\n";
            }
            container_cpp << "}\n\n";


            container_cpp << "void save(const " << container.name << " & value, Json::Value & json)\n";
            container_cpp << "{\n";
            for (auto& property : container.properties)
            {
                if (property.attribute_not_serializable)
                    continue;

                container_cpp << "    save(value." << property.name << ", json[\"" << property.name << "\"]" << ");\n";
            }          
            container_cpp << "}\n\n";
        }
    }

    void generate_events_data()
    {
        // --------------------------------------- all_containers.h -----------------------------------------
        auto& event_h = create_write_file("../poly/data/events/event.h");
        event_h << header_comment << "#pragma once\n";
        event_h << "#include \"types.h\"\n\n";

        event_h << "template <event_type type, typename... args>\n";
        event_h << "constexpr void broadcast_event(args && ...arguments)\n";
        event_h << "{\n";

        for (auto& event : events)
        {
            const auto& event_name = event.first;
            event_h << "    if constexpr (type == " << event_name << ")\n";
            event_h << "    {\n";
            for (auto& component_functions : event.second.component_funtions)
            {
                const auto& component_name = component_functions.first;
                const auto& functions = component_functions.second;
                event_h << "        for(auto& comp : " << component_name << "::array)\n";
                event_h << "        {\n";
                for (auto& function : functions)
                {
                    event_h << "            comp." << function << "(std::forward<args>(arguments)...);\n";
                }
                event_h << "        }\n";
            }
            event_h << "    }\n";
        }

        event_h << "}\n";
    }

    void join_all_cpp_files()
    {
        // overwrite or create data.h
        auto& data_h = create_write_file("../poly/data/data.cpp");

        // always needed content
        data_h << header_comment;

        // do not include the last one, since it the cpp we are writing on
        for (auto i = 0; i < cpp_files.size() - 1; i++)
        {
            data_h << "#include \"" << cpp_files[i] << "\"\n";
        }
    }

    void create_data_folders()
    {
        CreateDirectory("../poly/data", nullptr);
        CreateDirectory("../poly/data/components", nullptr);
        CreateDirectory("../poly/data/resources", nullptr);
        CreateDirectory("../poly/data/enums", nullptr);
        CreateDirectory("../poly/data/containers", nullptr);
        CreateDirectory("../poly/data/events", nullptr);
    }

    void write_to_files()
    {
        for (auto & write_file : write_files)
        {
            std::ifstream infile{ write_file.pathname, std::ios_base::in };
            std::string file_contents { std::istreambuf_iterator<char>(infile), std::istreambuf_iterator<char>() };
            infile.close();

            // do not overwrite if the contents are the same
            if (file_contents != write_file.text)
            {
                std::ofstream outfile{ write_file.pathname, std::ios_base::out };
                outfile << write_file.text;
                outfile.close();
            }
        }
    }

    write_file & create_write_file(const std::string & pathname)
    {
        // check if the file is a cpp
        if (pathname.substr(pathname.rfind(".") + 1) == "cpp")
        {
            // remove the path
            auto name = pathname;
            const auto slash_pos = pathname.rfind("/");
            if (slash_pos != std::string::npos)
                name = pathname.substr(slash_pos + 1);

            cpp_files.emplace_back(name);
        }
        write_files.emplace_back();
        auto & write_file = write_files.back();
        write_file.pathname = pathname;
        return write_file;
    }
}