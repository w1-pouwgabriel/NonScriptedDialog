# NonScriptedDialog

## Purpose

Traditional NPC dialogue is scripted: every line, branch, and response is hand-written ahead of time. That makes NPCs predictable, but also rigid. Players quickly learn there's nothing to "discover" in a conversation, and every new line of dialogue is manual authoring work that doesn't scale.

This project explores the alternative: NPCs that generate their responses live, in character, using a locally-run LLM (Qwen2.5) instead of pre-written dialogue trees. Each NPC is defined by a lightweight persona (a "character sheet" — background, personality, speech style, knowledge boundaries) rather than a script, and responds to whatever the player actually types, while staying in character and within the bounds the designer set.

The core technical problem this solves: running a language model inside a game in real time, shared efficiently across multiple NPCs, without tanking performance or needing a cloud API.

---

## Prerequisites

- **Unreal Engine 5.7**
- **LlamaCore** plugin ([Llama-Unreal](https://github.com/getnamo/Llama-Unreal) by getnamo) — enabled in `NonScriptedDialog.uproject`. Provides the local LLM runtime (`ULlamaSubsystem`).
- **A GGUF model file** — developed and tested against **Qwen2.5-1.5B-Instruct** at various quantization levels (q2_k through fp16). Place the file where `UNPCDialogSubsystem::Initialize()` expects it, or update the path there.
- Enough VRAM/RAM headroom for your chosen quantization — the smallest tested (q2_k) is ~750MB, the largest (fp16) is ~3.5GB.

---

## Modes

The project runs in a few distinct modes depending on what you're doing:

- **Normal gameplay** — the default. Walk up to an NPC, trigger an overlap, and `OpenDialogueUI()` opens the chat window. The player types freely; input switches to UI-only while the window is open, and back to normal game input on close.
- **Chat window (in-game UI)** — a UMG widget (`UNPCChatWidget`) showing scrolling conversation history above a text input field below, with responses streamed in token-by-token. Conversation history persists to disk per NPC and reloads automatically next session.
- **Benchmark sweep mode** — a debug/dev mode (triggered via a keybind, e.g. F9) that automatically loads every `.gguf` quantization in sequence, fires a fixed battery of test prompts at each, and logs timing/throughput data to a CSV for comparing model speed. Not part of normal play — used for picking the final shipped model.

Every NPC shares **one** loaded model instance (not one per character) — all requests are queued and processed one at a time through a central dialogue subsystem, regardless of which mode triggered them.
