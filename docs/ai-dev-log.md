# AI Development Log

Running log of AI use for CW1. Add an entry in the same commit as the work it relates to.

## Tools used
- Claude (Anthropic)
- ChatGPT (OpenAI)

## Entry template
**Date / Week:**
**Tool:**
**Category:** (code generation, debugging, documentation, event text, asset creation, balancing, etc.)
**Prompt (representative):**
**What the AI produced:**
**Verification:** how I reviewed it, issues found
**Changes I made:**
**Commit(s):**

---

## Entries

### Week 2: Project skeleton
**Tool:** ChatGPT
**Category:** Code commands, planning
**Prompt (representative):** Helped me set up the code commands to push my work to GitHub.
**What the AI produced:** The code commands required to compile the code and push it onto GitHub.
**Verification:** *1. Open Visual Studio Terminal
In Visual Studio:

View → Terminal

Then navigate to the folder containing your .sln file:

cd "C:\PATH\TO\YOUR\PROJECT"
Replace the path with your actual project folder.

2. Run these commands
git init -b main
git remote add origin https://github.com/AlexanderTovar765/ARTComp3016-Scarygon-Trail.git
git add .
git status
At this point, check the output. If it looks like it's listing your project files and isn't showing huge generated folders such as .vs, bin, or obj, continue:

git commit -m "Initial upload of Scarygon Trail"
git push -u origin main
⚠️ If git push gives an error
Don't run git push --force.

If you get an error such as:

rejected
non-fast-forward
or:

remote contains work that you do not have locally
paste the entire error message here and I'll give you the exact next command.

If you haven't created the .gitignore yet, tell me whether this is a C#, C++, Unity, or Unreal project before you run git add ., and I'll give you a copy-paste .gitignore setup appropriate for it.
*
**Changes I made:** *None, I knew how to open the terminal but I copied the commands as is and they uploaded without issue*
**Commit(s):** *(NA)*
