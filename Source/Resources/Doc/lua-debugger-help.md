# Using the Lua Debugger

CtrlrX includes a command-line style debugger for Lua (based on *clidebugger*). It lets you freeze a running Lua method at a point you choose, look at variables and the call stack, change values, and step through your code one line at a time.

The debugger lives in the **Debugger output** tab at the bottom of the Lua Method Editor.

---

## 1. Quick start

The debugger does nothing until your code asks for it. To start a debugging session, add a `pause()` call to your Lua method:

```lua
function Test_1()
    local a = 5
    local t = { name = "x", n = 3 }

    pause("before the calculation")     -- the script stops here

    local b = a * 2
    local c = b + t.n
    return c
end
```

Then run the method (from a button, a modulator, or by typing `Test_1()` in the Console tab). When execution reaches `pause()`:

1. The **Debugger output** tab comes to the front and prints `Paused at file test_1 line N` and your message.
2. The script is **frozen**. It waits for you.
3. The **Variables** pane (right) lists your local variables, and the **Stack trace** pane shows where you are.
4. Use the toolbar buttons or type commands in the input line at the bottom of the tab.
5. Press **Continue** when you want the script to carry on.

> The debugger reports the line that is **about to run next**, which is the first line after the `pause()` call.

---

## 2. The toolbar

Hover over a button to see its tooltip.

| Button (left to right) | What it does | Command it sends |
|---|---|---|
| Continue (▶) | Run until the next `pause()`, breakpoint or watch fires | `run` |
| Step Over | Run the next line, treating function calls as one step | `over` |
| Step Into | Run the next line, entering any function it calls | `step` |
| Step Out | Run until the current function returns to its caller | `out` |
| Restart | Stop debugging and let the script finish | `exit` |
| Stop | Same as Restart | `exit` |
| Clear (broom) | Clears the output text (does not affect the session) | none |

The buttons only work while the script is paused. At other times they do nothing.

**You can also type any command** in the input line at the bottom of the tab and press Enter. Typed commands are echoed in the output pane.

---

## 3. Panes

- **Output (left):** everything the debugger prints, including your typed commands and their results.
- **Variables (right):** the local variables (and upvalues) at the current stop. Tables show as `[N]`, the number of entries. To look inside a table, type `dump name 2`.
- **Stack trace (right):** the chain of function calls that led to the current line. Click its header bar to expand it.

**Not shown in the Variables pane:** boolean and function values are not listed. To check one, type `=myFlag` in the input line.

---

## 4. Commands

Type these in the input line. Arguments in `[ ]` are optional.

### Running and stepping

| Command | Meaning |
|---|---|
| `run` | Continue until the next breakpoint, watch or `pause()` |
| `step [N]` | Run N lines (default 1), stepping into function calls |
| `over [N]` | Run N lines (default 1), stepping over function calls |
| `out [N]` | Run until N functions have returned (default 1) |
| `gotoo line file` | Run until a given line in a given file |
| `exit` | End this debugging session |

### Looking at your program

| Command | Meaning |
|---|---|
| `vars [depth]` | List local variables and upvalues. Depth `0` means as deep as possible |
| `dump name [depth]` | Show a variable in full. For tables, name fields like `t.n` or `t.1.f` |
| `trace` | Print the call stack |
| `set [level]` | Choose which stack level `vars` and expressions refer to (levels come from `trace`) |
| `locs` / `ups` | List the names of locals / upvalues |
| `glob [depth]` | List global variables |
| `fenv [depth]` | List the function's environment |
| `show [line file X Y]` | Print the source around a line (X lines before, Y after) |
| `what func` | Show where a function is defined, if known |
| `info` | Dump everything the debugger knows (very large; for diagnosing the debugger itself) |

### Evaluating and changing things

Anything that isn't a command is run as Lua in the current context:

```
=a * 2                 -- prints 10   ("=" is shorthand for "return")
=t.name                -- prints x
a = 99                 -- changes the variable immediately
=string.format("%d", a)
```

Changes you make take effect at once, so you can test a fix without editing and re-running.

### Breakpoints and watches

| Command | Meaning |
|---|---|
| `setb line file` | Stop whenever this line is about to run. Line `0` means any line in the file |
| `delb line file` | Remove a breakpoint |
| `listb` / `delallb` | List / remove all breakpoints |
| `setw expression` | Stop whenever the expression becomes true, e.g. `setw a > 10` |
| `delw index` / `delallw` | Remove a watch / all watches |
| `listw` | List watches |

**File names:** the file is the method's name. Partial names are fine, and the extension can be left off. On Windows the debugger uses **lowercase** names, so a method called `Test_1` appears as `test_1`:

```
setb 7 test_1
run
```

### Pausing control

| Command | Meaning |
|---|---|
| `poff` | Ignore all `pause()` calls from now on |
| `pon` | Honour `pause()` calls again |
| `tron [crl]` | Print a line for every call (`c`), return (`r`) and/or line (`l`). `tron` alone turns tracing off |
| `help [command]` | List the commands, or explain one |

---

## 5. `pause()` in detail

```lua
pause("message")            -- stop here, show the message
pause("message", 5)         -- stop after 5 more lines have run
pause("message", nil, true) -- force a stop even after "poff"
```

Put `pause()` wherever you want to start looking: at the top of a function you're investigating, inside an `if`, or just before the line that misbehaves. Several `pause()` calls in one method are fine; each one stops in turn when you press Continue.

You can also type `pause()` in the **Console** tab to start a session without editing any code.

---

## 6. Tips and things to know

- **Everything freezes while paused.** The script waits for you, and the rest of the application won't accept clicks. Windows may play an error beep if you click elsewhere. This is normal. Press **Continue** or **Stop** first, and then you can close windows again.
- **Don't leave `pause()` calls in a panel you give to other people.** They would freeze the panel for anyone who runs it. Guard them with a switch so you can turn them off:
  ```lua
  DEBUGGING = true                     -- set to false before distributing
  if DEBUGGING then pause("here") end
  ```
- **`exit` ends the session, but not future ones.** The next `pause()` that runs starts a new session. Use `poff` if you want pauses ignored.
- **Press Stop (or type `exit`) when you are finished.** After a session the debugger stays attached and slows Lua down until you do.
- **Be careful in time-critical callbacks.** Pausing inside something that runs constantly (`onMidiReceived`, timers, buttons that are held down) freezes that processing while you look, and events can pile up. Put `pause()` in code that runs on demand where you can.
- **Line numbers** are the ones the debugger prints. If they seem a few lines off compared to the editor, use `show` to see exactly which line the debugger means.
- **Anything that calls `debug.traceback()` also starts a session.** The debugger hooks that standard function, so if an error handler in your code (or an `assert` that builds a traceback) calls it, the debugger opens at that point. Type `exit` or press **Stop** to carry on. Ordinary runtime errors are otherwise not intercepted. It stops at `pause()` calls, breakpoints and watches.

---

## 7. Troubleshooting

| Problem | What to try |
|---|---|
| `attempt to call global 'pause' (a nil value)` | The debugger script isn't loaded in this panel. Reload the panel, and make sure the *Built-In* method group is present. |
| Nothing happens when I press a toolbar button | The script isn't paused. Buttons only act while a `pause()`, breakpoint or watch has stopped it. |
| `vars` is empty | The function has no local variables, or none are non-nil yet. Use `glob` for globals. |
| I can't click or close anything | A session is waiting. Press **Continue** or **Stop**. |
| The panel is slow after debugging | A session is still attached. Press **Stop**. |
| Breakpoint never triggers | Check the file name is lowercase and the method name is spelled correctly (`listb` shows what is set). |


