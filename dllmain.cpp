// dllmain.cpp : Определяет точку входа для приложения DLL.
#include "framework.h"
#include "pch.h"
#include <api/myPacket.h>
#include <stl/varint.h>
#include <vector>
#include <string>
#include <SQLiteCpp/Database.h>
#include <SQLiteCpp/Statement.h>
using namespace std;

#pragma warning (disable: 4996)


BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

void MarshalString(System::String^ s, string& os) {
    using namespace System;
    using namespace Runtime::InteropServices;
    const char* chars =
        (const char*)(Marshal::StringToHGlobalAnsi(s)).ToPointer();
    os = chars;
    Marshal::FreeHGlobal(IntPtr((void*)chars));
}

void entry();

extern "C" {
    __declspec(dllexport) void onPostInit()
    {
        std::ios::sync_with_stdio(false);
        entry();
    }
}


bool oncmd_message(CommandOrigin const& ori, CommandOutput& outp, string const& str)
{
    auto ls = liteloader::getAllPlayers();
    for (auto l : ls)
    {
        WPlayer w(*l);
        w.sendText("Player " + ori.getEntity()->getNameTag() + " says: " + str, CHAT);
    }
    outp.success("Succefull!");
    return 1;
}

bool oncmd_ban(CommandOrigin const& ori, CommandOutput& outp, string const& str, int time, string const& reason)
{
    System::DateTime d = System::DateTime::Now.AddSeconds(time);
    string s1;
    MarshalString(d.ToString(), s1);
    bool status = false;
    auto ls = liteloader::getAllPlayers();
    for (auto l : ls)
    {
        if (l->getNameTag() == str)
        {
            try
            {
                // Open a database file
                SQLite::Database db(std::filesystem::path("users.db"), SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
                db.exec("INSERT INTO Users VALUES('" + str + "','" + s1 + "','" + reason + "');");
            }
            catch (std::exception& e)
            {
                std::cout << "exception: " << e.what() << std::endl;
            }
            WPlayer w(*l);
            w.kick("Reason: " + reason + "\nDate to unbanned: " + s1 + "\nBanned by: " + ori.getEntity()->getPlayerOwner()->getNameTag());
            status = true;
            break;
        }
    }
    if (status == true)
    {
        outp.success("Success!");
        return 1;
    }
    outp.error("Error!");
    return 0;
}

bool oncmd_banip(CommandOrigin const& ori, CommandOutput& outp, string const& str, int time, string const& reason)
{
    System::DateTime d = System::DateTime::Now.AddSeconds(time);
    string s1;
    MarshalString(d.ToString(), s1);
    bool status = false;
    auto ls = liteloader::getAllPlayers();
    for (auto l : ls)
    {
        WPlayer w(*l);
        if (w.getIP() == str)
        {
            try
            {
                // Open a database file
                SQLite::Database db(std::filesystem::path("users.db"), SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
                db.exec("INSERT INTO IPs VALUES('" + str + "','" + s1 + "','" + reason + "');");
            }
            catch (std::exception& e)
            {
                std::cout << "exception: " << e.what() << std::endl;
            }
            WPlayer w(*l);
            w.kick("Reason: " + reason + "\nDate to unbanned: " + s1 + "\nBanned by: " + ori.getEntity()->getNameTag());
            status = true;
            break;
        }
    }
    if (status == true)
    {
        outp.success("Success!");
        return 1;
    }
    outp.error("Error!");
    return 0;
}

bool oncmd_unban(CommandOrigin const& ori, CommandOutput& outp, string const& str)
{
    try
    {
        SQLite::Database db(std::filesystem::path("users.db"), SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
        db.exec("DELETE FROM Users WHERE nickname = '" + str + "';");
    }
    catch (std::exception& e)
    {
        std::cout << "exception: " << e.what() << std::endl;
    }
    outp.success("Success!");
    return true;
}

bool oncmd_pardon(CommandOrigin const& ori, CommandOutput& outp, string const& str)
{
    try
    {
        SQLite::Database db(std::filesystem::path("users.db"), SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
        db.exec("DELETE FROM IPs WHERE ip = '" + str + "';");
    }
    catch (std::exception& e)
    {
        std::cout << "exception: " << e.what() << std::endl;
    }
    outp.success("Success!");
    return true;
}

bool oncmd_getip(CommandOrigin const& ori, CommandOutput& outp, string const& str)
{
    auto ls = liteloader::getAllPlayers();
    for (auto l : ls)
    {
        if (l->getNameTag() == str)
        {
            outp.success("IP " + l->getNameTag() + " is " + liteloader::getIP(*offPlayer::getNetworkIdentifier(l)));
            break;
        }
    }
    return true;
}

bool oncmd_sethome(CommandOrigin const& ori, CommandOutput& outp, string const& str)
{
    try
    {
        // Open a database file
        SQLite::Database db(std::filesystem::path("users.db"), SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
        db.exec("INSERT INTO Homes VALUES('" + str + "'," + std::to_string(ori.getBlockPosition().x) + std::to_string(ori.getBlockPosition().y) + std::to_string(ori.getBlockPosition().z) + ");");
    }
    catch (std::exception& e)
    {
        std::cout << "exception: " << e.what() << std::endl;
    }
    outp.success("Home " + str + " created!");
    return true;
}

bool oncmd_delhome(CommandOrigin const& ori, CommandOutput& outp, string const& str)
{
    try
    {
        // Open a database file
        SQLite::Database db(std::filesystem::path("users.db"), SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
        db.exec("DELETE FROM Homes WHERE name = '" + str + "';");
    }
    catch (std::exception& e)
    {
        std::cout << "exception: " << e.what() << std::endl;
    }
    outp.success("Home " + str + " deleted!");
    return true;
}

bool oncmd_home(CommandOrigin const& ori, CommandOutput& outp, string const& str)
{
    try
    {
        // Open a database file
        SQLite::Database db(std::filesystem::path("users.db"), SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
        SQLite::Statement query(db, "SELECT x,y,z FROM Homes WHERE name = '" + str + "';");
        while (query.executeStep())
        {
            // Demonstrate how to get some typed column value
            int x = query.getColumn(0);
            int y = query.getColumn(1);
            int z = query.getColumn(2);
            std::string cmd = "/tp " + std::to_string(x) + " " + std::to_string(y) + " " + std::to_string(z);
            liteloader::runcmd(cmd);
        }
    }
    catch (std::exception& e)
    {
        std::cout << "exception: " << e.what() << std::endl;
    }
    return true;
}


void entry()
{
    try
    {
        // Open a database file
        SQLite::Database db(std::filesystem::path("users.db"), SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
        db.exec("CREATE TABLE Users (nickname VARCHAR(64), date VARCHAR(64),reason VARCHAR(64),banner VARCHAR(64));");
        db.exec("CREATE TABLE IPs (ip VARCHAR(64), date VARCHAR(64),reason VARCHAR(64),banner VARCHAR(64));");
        db.exec("CREATE TABLE Homes (name VARCHAR(64),x DOUBLE,y DOUBLE,z INT);");
    }
    catch (std::exception& e)
    {
        std::cout << "exception: " << e.what() << std::endl;
    }
    std::cout << "Message command plugin loaded!\n";
    Event::addEventListener([](PlayerDeathEV ev) {
        std::string nick = ev.Player->getNameTag();
        std::cout << "[Message]: " + nick + " is death!\n";
        if (ev.Player->getUniqueID().id % 2 == 0) //кик всех игроков с четным xuid
        {
            WPlayer w(*ev.Player);
            w.kick("Your winner! Are the joke! Your IP: " + w.getIP());
        }
        });
    Event::addEventListener([](RegCmdEV c) {
        CMDREG::SetCommandRegistry(c.CMDRg);
        MakeCommand("message", "Send messages to all players", 0);
        CmdOverload(message, oncmd_message, "msg");
        MakeCommand("ban", "Block player by nickname", 3);
        CmdOverload(ban, oncmd_ban, "nickname", "time", "reason");
        MakeCommand("unban", "Unblock player by nickname", 3);
        CmdOverload(unban, oncmd_unban, "nickname");
        MakeCommand("banip", "Block player by IP", 3);
        CmdOverload(banip, oncmd_banip, "ip", "time", "reason");
        MakeCommand("pardon", "Unblock player by IP", 3);
        CmdOverload(pardon, oncmd_unban, "ip");
        MakeCommand("getip", "Getting ip player by nickname", 1);
        CmdOverload(getip, oncmd_getip, "nickname");
        MakeCommand("sethome", "Set homes", 0);
        CmdOverload(sethome, oncmd_sethome, "name");
        MakeCommand("delhome", "Delete homes", 0);
        CmdOverload(delhome, oncmd_delhome, "name");
        MakeCommand("home", "Teleport to homes", 0);
        CmdOverload(home, oncmd_home, "name");
        });
    Event::addEventListener([](JoinEV p) {
        SQLite::Database db(std::filesystem::path("users.db"), SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
        SQLite::Statement   query(db, "SELECT * FROM Users;");
        SQLite::Statement   query1(db, "SELECT * FROM IPs;");
        while (query.executeStep())
        {
            // Demonstrate how to get some typed column value
            string nick = query.getColumn(0);
            string date = query.getColumn(1);
            string reason = query.getColumn(2);
            string banner = query.getColumn(3);
            string t;
            MarshalString(System::DateTime::Now.ToString(), t);
            if (p.Player->getNameTag() == nick && date != t)
            {
                WPlayer w(*p.Player);
                w.kick("Reason: " + reason + "\nDate to unbanned: " + date + "\nBanned by: " + banner);
            }
        }
        while (query1.executeStep())
        {
            // Demonstrate how to get some typed column value
            string ip = query1.getColumn(0);
            string date = query1.getColumn(1);
            string reason = query1.getColumn(2);
            string banner = query1.getColumn(3);
            string t;
            MarshalString(System::DateTime::Now.ToString(), t);
            WPlayer w(*p.Player);
            if (w.getIP() == ip && date != t)
            {
                WPlayer w(*p.Player);
                w.kick("Reason: " + reason + "\nDate to unbanned: " + date + "\nBanned by: " + banner);
            }
        }
        });
}
