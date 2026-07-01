#!/usr/bin/env python3
"""
test_ircserv.py — automated smoke test for a 42 ft_irc server.

Focuses on cases that are awkward to trigger manually with HexChat/irssi:
  - registration order / bad password handling
  - partial reads (command split across two TCP writes)
  - multiple commands sent in a single TCP write (incl. QUIT + another command,
    which is the use-after-free scenario worth regression-testing)
  - basic multi-client JOIN / PRIVMSG / PART / QUIT flow
  - rapid connect/disconnect (fd reuse stress)

Usage:
    python3 test_ircserv.py <host> <port> <password>

Example:
    python3 test_ircserv.py localhost 6667 mypass

Each test prints PASS/FAIL/INFO. Nothing here asserts exact numeric-reply
formatting (server implementations vary slightly) — it mainly checks that
the server responds, doesn't hang, and doesn't crash/close unexpectedly.
After running, also check the server process is still alive and re-run
under valgrind separately for leak/fd checks.
"""

import socket
import sys
import time

TIMEOUT = 2.0


def connect(host, port):
    s = socket.create_connection((host, port), timeout=TIMEOUT)
    s.settimeout(TIMEOUT)
    return s


def send(sock, line):
    sock.sendall((line + "\r\n").encode())


def send_raw(sock, raw_bytes):
    sock.sendall(raw_bytes)


def recv_all(sock, wait=0.3):
    """Drain whatever is available within `wait` seconds. Returns bytes.
    Treats a connection reset the same as a clean close (both just mean
    "nothing more to read"), so callers don't need their own try/except."""
    sock.settimeout(wait)
    chunks = []
    try:
        while True:
            data = sock.recv(4096)
            if not data:
                break
            chunks.append(data)
    except socket.timeout:
        pass
    except (ConnectionResetError, BrokenPipeError, OSError):
        pass
    return b"".join(chunks)


def register(sock, password, nick, user):
    send(sock, f"PASS {password}")
    send(sock, f"NICK {nick}")
    send(sock, f"USER {user} 0 * :{user} Realname")
    time.sleep(0.2)
    return recv_all(sock)


def ok(msg):
    print(f"  [PASS] {msg}")


def fail(msg):
    print(f"  [FAIL] {msg}")


def info(msg):
    print(f"  [INFO] {msg}")


def test_wrong_password(host, port, password):
    print("\n=== TEST: wrong password disconnects client ===")
    s = connect(host, port)
    send(s, f"PASS {password}wrong")
    time.sleep(0.3)
    resp = recv_all(s)
    info(f"server said: {resp!r}")
    if b"464" not in resp and b"incorrect" not in resp.lower():
        fail("no password-mismatch error seen")
    # server should have closed the connection; a subsequent send should fail
    # or be silently discarded (both are acceptable — the point is no further
    # command gets processed as if the client were authenticated)
    try:
        s.sendall(b"NICK afterreject\r\n")
        time.sleep(0.2)
        after = recv_all(s)
        if after == b"":
            ok("connection closed by server after bad password (no further response)")
        else:
            fail(f"server responded to a command after bad password: {after!r}")
    except (ConnectionResetError, BrokenPipeError, OSError):
        ok("connection closed by server after bad password")
    finally:
        s.close()


def test_registration_order(host, port, password):
    print("\n=== TEST: registration before PASS is rejected ===")
    s = connect(host, port)
    send(s, "NICK earlynick")
    time.sleep(0.2)
    resp = recv_all(s)
    info(f"server said: {resp!r}")
    if b"451" in resp or b"NOTREGISTERED" in resp.upper() or resp != b"":
        ok("server responded to premature NICK (check it's an error, not success)")
    else:
        fail("no response to NICK sent before PASS")
    s.close()

    print("\n=== TEST: correct PASS/NICK/USER order registers successfully ===")
    s = connect(host, port)
    resp = register(s, password, "goodnick", "gooduser")
    info(f"server said: {resp!r}")
    if b"001" in resp or b"Welcome" in resp:
        ok("received welcome / 001 reply")
    else:
        fail("no welcome reply seen after full registration")
    s.close()


def test_partial_read(host, port, password):
    print("\n=== TEST: command split across two TCP writes ===")
    s = connect(host, port)
    resp = register(s, password, "partial1", "partialuser")
    if b"001" not in resp:
        fail(f"registration failed before partial-read test even started: {resp!r}")
        s.close()
        return

    line = "PRIVMSG partial1 :hello world\r\n"
    half = len(line) // 2
    send_raw(s, line[:half].encode())
    time.sleep(0.3)   # force it to arrive as a separate TCP segment / recv() call
    send_raw(s, line[half:].encode())
    time.sleep(0.3)
    resp = recv_all(s)
    info(f"server said: {resp!r}")
    # sending PRIVMSG to yourself should not crash the server; response may or
    # may not include the message depending on server's self-message policy
    ok("server survived split command (check process still alive separately)")
    s.close()


def test_multi_command_single_packet(host, port, password):
    print("\n=== TEST: QUIT + another command in a single write (UAF regression) ===")
    s = connect(host, port)
    register(s, password, "quitnick", "quituser")
    recv_all(s)

    payload = b"QUIT :bye\r\nPRIVMSG quitnick :should not crash server\r\n"
    send_raw(s, payload)
    time.sleep(0.3)
    try:
        resp = recv_all(s)
        info(f"server said: {resp!r}")
    except OSError:
        pass
    s.close()

    # if the server is still alive and accepting connections, we consider this a pass
    try:
        s2 = connect(host, port)
        send(s2, f"PASS {password}")
        send(s2, "NICK aliveping")
        send(s2, "USER aliveping 0 * :Alive Ping")
        time.sleep(0.2)
        resp2 = recv_all(s2)
        if b"001" in resp2 or resp2 != b"":
            ok("server still alive and responsive after QUIT+command-in-one-packet")
        else:
            fail("server unresponsive after QUIT+command-in-one-packet")
        s2.close()
    except OSError as e:
        fail(f"could not reconnect to server after test — it may have crashed: {e}")


def test_multi_client_flow(host, port, password):
    print("\n=== TEST: two clients, JOIN / PRIVMSG / PART / QUIT ===")
    a = connect(host, port)
    b = connect(host, port)
    register(a, password, "alice", "alice")
    register(b, password, "bob", "bob")
    recv_all(a)
    recv_all(b)

    send(a, "JOIN #testchan")
    time.sleep(0.2)
    resp_a = recv_all(a)
    info(f"alice JOIN response: {resp_a!r}")

    send(b, "JOIN #testchan")
    time.sleep(0.2)
    resp_a2 = recv_all(a)   # alice should see bob's JOIN broadcast
    resp_b = recv_all(b)
    info(f"alice sees bob join: {resp_a2!r}")
    info(f"bob JOIN response: {resp_b!r}")
    if b"bob" in resp_a2 and b"JOIN" in resp_a2:
        ok("alice received bob's JOIN broadcast")
    else:
        fail("alice did not see bob's JOIN broadcast")

    send(b, "PRIVMSG #testchan :hello from bob")
    time.sleep(0.2)
    resp_a3 = recv_all(a)
    info(f"alice sees bob's message: {resp_a3!r}")
    if b"hello from bob" in resp_a3:
        ok("channel PRIVMSG delivered correctly")
    else:
        fail("channel PRIVMSG not received by alice")

    send(b, "PART #testchan :leaving now")
    time.sleep(0.2)
    resp_a4 = recv_all(a)
    info(f"alice sees bob's PART: {resp_a4!r}")
    if b"PART" in resp_a4:
        ok("PART broadcast received")
    else:
        fail("PART broadcast not received")

    send(a, "QUIT :done testing")
    send(b, "QUIT :done testing")
    time.sleep(0.2)
    a.close()
    b.close()


def test_rapid_connect_disconnect(host, port, password, rounds=20):
    print(f"\n=== TEST: rapid connect/disconnect x{rounds} (fd reuse stress) ===")
    for i in range(rounds):
        nick = f"rp{i}"  # keep well under 9 chars even at rounds=99
        try:
            s = connect(host, port)
            send(s, f"PASS {password}")
            send(s, f"NICK {nick}")
            send(s, f"USER {nick} 0 * :Rapid {i}")
            time.sleep(0.05)
            recv_all(s, wait=0.1)
            s.close()
        except OSError as e:
            fail(f"connection {i} failed: {e}")
            return
    # final check: server still accepts new connections after the burst,
    # and actually completes registration (not just "responded with something")
    try:
        s = connect(host, port)
        send(s, f"PASS {password}")
        send(s, "NICK aftrbrst")
        send(s, "USER aftrbrst 0 * :After Burst")
        time.sleep(0.2)
        resp = recv_all(s)
        if b"001" in resp:
            ok(f"server survived {rounds} rapid connect/disconnects and still registers clients correctly")
        else:
            fail(f"server did not complete registration after burst: {resp!r}")
        s.close()
    except OSError as e:
        fail(f"server did not accept connection after burst: {e}")


def test_case_insensitive_nickname(host, port, password):
    print("\n=== TEST: case-insensitive nickname matching ===")

    # Part 1: NICK collision should be detected regardless of case
    a = connect(host, port)
    resp = register(a, password, "CaseTest", "caseuser")
    if b"001" not in resp:
        fail(f"setup registration failed: {resp!r}")
        a.close()
        return

    b = connect(host, port)
    send(b, f"PASS {password}")
    send(b, "NICK casetest")   # same nick, different case
    time.sleep(0.2)
    resp_b = recv_all(b)
    info(f"second client NICK casetest -> {resp_b!r}")
    if b"433" in resp_b or b"NICKNAMEINUSE" in resp_b.upper() or b"in use" in resp_b.lower():
        ok("NICK collision detected case-insensitively (CaseTest vs casetest)")
    else:
        fail(f"nickname collision NOT detected across case — server allowed 'casetest' "
             f"while 'CaseTest' is taken: {resp_b!r}")

    # give b a real distinct nick so the second part of the test can proceed
    send(b, "NICK distinct1")
    send(b, "USER distinctuser 0 * :Distinct User")
    time.sleep(0.2)
    resp_b2 = recv_all(b)
    if b"001" not in resp_b2:
        fail(f"could not register second client with a distinct nick: {resp_b2!r}")
        a.close()
        b.close()
        return

    # Part 2: PRIVMSG targeting should resolve regardless of nick case
    send(b, "PRIVMSG CASETEST :hello via uppercase target")
    time.sleep(0.2)
    resp_a = recv_all(a)
    resp_b3 = recv_all(b)
    info(f"registrant 'CaseTest' receives (targeted as CASETEST): {resp_a!r}")
    if b"hello via uppercase target" in resp_a:
        ok("PRIVMSG resolved target nick case-insensitively (CASETEST -> CaseTest)")
    elif b"401" in resp_b3 or b"No such nick" in resp_b3:
        fail("PRIVMSG to 'CASETEST' failed to resolve to registered nick 'CaseTest' "
             "(nickname lookup appears case-sensitive)")
    else:
        fail(f"unexpected result for cross-case PRIVMSG: a saw {resp_a!r}, b saw {resp_b3!r}")

    send(a, "QUIT :done")
    send(b, "QUIT :done")
    time.sleep(0.2)
    a.close()
    b.close()


def test_mode_mixed_signs(host, port, password):
    print("\n=== TEST: MODE with mixed +/- flags in one command ===")
    a = connect(host, port)
    b = connect(host, port)
    resp_a = register(a, password, "modeop", "modeop")
    resp_b = register(b, password, "modetrg", "modetrg")
    if b"001" not in resp_a or b"001" not in resp_b:
        fail(f"setup registration failed: a={resp_a!r} b={resp_b!r}")
        a.close()
        b.close()
        return

    send(a, "JOIN #modechan")
    time.sleep(0.2)
    recv_all(a)

    send(b, "JOIN #modechan")
    time.sleep(0.2)
    recv_all(a)
    recv_all(b)

    # modeop is the channel creator -> operator. Apply a mixed +/- mode string
    # in one command: set invite-only, unset topic-restricted, grant operator
    # to modetrg, all in a single MODE line.
    send(a, "MODE #modechan +i-t+o modetrg")
    time.sleep(0.3)
    resp_a2 = recv_all(a)
    resp_b2 = recv_all(b)
    info(f"MODE broadcast seen by modeop: {resp_a2!r}")
    info(f"MODE broadcast seen by modetrg: {resp_b2!r}")

    combined = resp_a2 + resp_b2
    if b"MODE #modechan" not in combined:
        fail(f"no MODE broadcast observed at all: {combined!r}")
    else:
        # Check every flag appears with a sign attached somewhere in the message,
        # not just trailing along under the wrong sign due to a shared 'adding' bug.
        has_plus_i = b"+i" in combined or b"i" in combined.split(b"MODE #modechan", 1)[-1].split(b"\r\n")[0]
        text_line = combined.split(b"MODE #modechan", 1)[-1].split(b"\r\n")[0]
        info(f"mode portion of broadcast: {text_line!r}")
        if b"+i" in text_line and b"-t" in text_line and b"+o" in text_line:
            ok("mixed +i-t+o correctly reflected with per-flag signs in broadcast")
        elif b"i" in text_line and b"t" in text_line and b"o" in text_line:
            fail(f"all three flags present but signs look collapsed/wrong: {text_line!r} "
                 f"(expect +i, -t, +o distinctly, not everything under one sign)")
        else:
            fail(f"MODE broadcast missing expected flags: {text_line!r}")

    # Sanity check the applied state via a bare MODE query
    send(a, "MODE #modechan")
    time.sleep(0.2)
    resp_a3 = recv_all(a)
    info(f"MODE query after change: {resp_a3!r}")
    if b"+i" in resp_a3 or b"i" in resp_a3:
        pass  # channel mode query format varies by implementation; informational only
    else:
        info("could not confirm invite-only state via MODE query (format may differ) — not a hard fail")

    send(a, "QUIT :done")
    send(b, "QUIT :done")
    time.sleep(0.2)
    a.close()
    b.close()


def main():
    if len(sys.argv) != 4:
        print(f"Usage: {sys.argv[0]} <host> <port> <password>")
        sys.exit(1)

    host, port, password = sys.argv[1], int(sys.argv[2]), sys.argv[3]

    print(f"Testing ft_irc server at {host}:{port}")
    print("Make sure the server is already running in another terminal.")
    print("For fd-leak/UAF detection, run the server under valgrind separately:")
    print("  valgrind --leak-check=full --track-fds=yes ./ircserv <port> <password>")

    tests = [
        test_wrong_password,
        test_registration_order,
        test_partial_read,
        test_multi_command_single_packet,
        test_multi_client_flow,
        test_rapid_connect_disconnect,
        test_case_insensitive_nickname,
        test_mode_mixed_signs,
    ]

    try:
        for test_fn in tests:
            try:
                test_fn(host, port, password)
            except ConnectionRefusedError:
                raise
            except Exception as e:
                fail(f"{test_fn.__name__} raised an unexpected exception: {e!r}")
    except ConnectionRefusedError:
        print("\nCould not connect — is the server running on that host/port?")
        sys.exit(1)

    print("\nDone. Review any [FAIL]/[INFO] lines above, and check the server's")
    print("own terminal output + a separate valgrind run for crashes/leaks.")


if __name__ == "__main__":
    main()