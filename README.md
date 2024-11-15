
# An Adventure in the World of Self-Debugging

This project presents an effective method for detecting debuggers through self-debugging. The technique uses the Windows API to apply a process to itself and identify potential debuggers. If an external debugger is already active, the self-debugging attempt will fail, and this detection can be leveraged to uncover debugging activities.

I'll be honest, I have no idea if it could be used in malware development, try it out and let me know.

---
## A Little Story About Big Debuggers

So, picture this: I was on the hunt for anti-debugging methods and stumbled upon some interesting articles about the WinAPI function `DebugActiveProcess`. This function allows a debugger to attach to an active process and debug it [1].

The problem? You can't debug yourself! Well, that put the WinAPI out of the game for now. But I didn't want to use the standard WinAPIs like `IsDebuggerPresent` and so on—where's the fun in that?

So I researched whether a thread could debug the main process. Answer: No! Did I lack the rights in the process (`SE_DEBUG_NAME`)? Doesn't have to be; it works without it. Nevertheless, I assign the privilege in the proof of concept code—you can decide for yourself whether to use it (nice, right?).

But wait a minute, what if I start a new process within my process? Is that legitimate or just plain genius? Well, let's find out!

---
## Why a Process Cannot Debug Itself

In Windows, a process cannot debug itself due to the design of the debugging APIs. When a process calls `DebugActiveProcess` on its own PID, the call fails because a process cannot act as both the debugger and the debuggee simultaneously. The operating system enforces this separation to maintain stability and prevent recursive debugging scenarios that could lead to deadlocks or inconsistent states. It's like trying to lift yourself up by your own bootstraps—it just doesn't work!

---
## Implementing a Helper Process

Since a process cannot debug itself, we can create a helper process to do the dirty work. Sounds like a plan, right?
### Steps

1. **Main Process**: Starts normally and retrieves its own process ID (PID).
2. **Privilege Escalation**: Optionally enables debug privileges using `AdjustTokenPrivileges`.
3. **Launching Helper Process**: Uses `CreateProcess` to start a new instance of the program with a `--debug` flag and passes its PID.
4. **Helper Process**: Checks for the `--debug` flag and attempts to attach to the main process using `DebugActiveProcess`.
5. **Debugging Actions**: The helper process performs anti-debugging checks and reports findings.

##### Helper Process Attaching to Main Process:

```c++
if (DebugActiveProcess(targetPID)) {
    wprintf(L"[+] Attached to target process %d.\n", targetPID);
    // Perform debugging actions
    DebugActiveProcessStop(targetPID);
} else {
    wprintf(L"[-] Failed to attach to target process. Error: %d\n", GetLastError());
    ExitProcess(1);
}
```

If all goes well, the helper process attaches to the main process, and they live happily ever after—until the end of their runtime. If not, well, there's an error message, and we try again. Or we grab a coffee first.

---

### Screenshot's 

**Process execution without debug privileges**:

Here we see how our process looks without the mighty debug privileges. Quite unassuming, isn't it?

![[Screenshot 2024-11-15 153215.png]]

**Process execution with those nice privileges**:

Now we've activated the debug privileges. Feels a lot more powerful already!

![[Screenshot 2024-11-15 153615.png]]



**Now we'll try it with a debugger (OMG, I'm so excited too! Haha):**

And behold! When an external debugger is active, our little trick fails, and we're in the know. It's almost like cheating—but in a good way!

![[Screenshot 2024-11-15 154244.png]]


---

## Conclusion

Who would have thought that anti-debugging could be such an adventure? By creating a helper process, we can cleverly circumvent the fact that a process cannot debug itself. It's like we've created a twin to handle the unpleasant tasks for us. Pretty clever, right?

So, the next time you want to outsmart a debugger, remember: sometimes you just need a little helper to achieve greatness.

---

## References:

[1] https://learn.microsoft.com/en-us/windows/win32/api/debugapi/nf-debugapi-debugactiveprocess
[2] https://learn.microsoft.com/en-us/windows/win32/secauthz/privilege-constants
[3] https://github.com/LordNoteworthy/al-khaser/tree/master
[4] https://0xpat.github.io/
[5] https://www.codeproject.com/Articles/30815/An-Anti-Reverse-Engineering-Guide
[6] https://maldevacademy.com/

To be honest, there is so much content and similar techniques that are always similar. These are the references you should definitely check out because there is so much to learn.

By the way, if you're looking to level up in this field, I’ve gotta say, I’m in the MalDev course right now and… WOW. (Not sponsored, no cash involved – just pure, mind-blowing knowledge, haha). You learn _so_ much, and I’m not even halfway through, but my brain already feels like it’s running out of storage space. The content? Absolutely top-notch. Hats off to the creators (shoutout to them!) – they really knocked it out of the park.

---
