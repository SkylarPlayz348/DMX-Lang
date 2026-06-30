# DMX-Lang

# **Very Much a Work In Progress**

DMX Lang is a compiled language with similar syntax to assembly language using the GNU Assembler. In no way do you need to know how to code to use this as most of the programming is done using defined commands which are referenced on the fly to all for the most portability possible.

All controllers treat midi differently and as such we don't depend on everything being compiled when the files get processed. That is why as per the DMX Lang Spec there is definitions you can make to make sure your controller fits perfectly. Some controllers are baked into the compiler and require no external work to compile but if some controller runs under my radar or isn't in my database you can make definitions yourself. Most manufactures have their manuals online so you can find the manual from there but if not there are tons of resources online that can also provide you with the manual. The Only things you need to know for a full definition of a command is what midi event is it and what note or notes does it need. The spec for a definition runs like this `<command> argument:midi event:note`. Some definitions like ones backed into the compiler may add functionality as per the spec every single command would need to be done individually and that is just tedious. So there are some

## Note:
While we vendor SDL3 using git submodules the dmx compiler and language itself will not contain any SDL3 code at all. It is purely meant for the Visualizer and planned editor.

## Credits:

Credits for this project are in the [CREDITS.md](./CREDITS.md) file