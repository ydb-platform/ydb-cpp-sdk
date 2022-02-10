""" 
Utilities for end-users. 
""" 
 
from __future__ import absolute_import 
import __main__ 
from collections import namedtuple 
import logging
import traceback
import re 
import os 
import sys 
 
from parso import split_lines

from jedi import Interpreter 
from jedi.api.helpers import get_on_completion_name
 
 
READLINE_DEBUG = False


def setup_readline(namespace_module=__main__): 
    """ 
    Install Jedi completer to :mod:`readline`. 
 
    This function setups :mod:`readline` to use Jedi in Python interactive 
    shell.  If you want to use a custom ``PYTHONSTARTUP`` file (typically 
    ``$HOME/.pythonrc.py``), you can add this piece of code:: 
 
        try: 
            from jedi.utils import setup_readline 
            setup_readline() 
        except ImportError: 
            # Fallback to the stdlib readline completer if it is installed. 
            # Taken from http://docs.python.org/2/library/rlcompleter.html 
            print("Jedi is not installed, falling back to readline") 
            try: 
                import readline 
                import rlcompleter 
                readline.parse_and_bind("tab: complete") 
            except ImportError: 
                print("Readline is not installed either. No tab completion is enabled.") 
 
    This will fallback to the readline completer if Jedi is not installed. 
    The readline completer will only complete names in the global namespace, 
    so for example:: 
 
        ran<TAB> 
 
    will complete to ``range`` 
 
    with both Jedi and readline, but:: 
 
        range(10).cou<TAB> 
 
    will show complete to ``range(10).count`` only with Jedi. 
 
    You'll also need to add ``export PYTHONSTARTUP=$HOME/.pythonrc.py`` to 
    your shell profile (usually ``.bash_profile`` or ``.profile`` if you use 
    bash). 
 
    """ 
    if READLINE_DEBUG:
        logging.basicConfig(
            filename='/tmp/jedi.log',
            filemode='a',
            level=logging.DEBUG
        )

    class JediRL(object): 
        def complete(self, text, state): 
            """ 
            This complete stuff is pretty weird, a generator would make 
            a lot more sense, but probably due to backwards compatibility 
            this is still the way how it works. 
 
            The only important part is stuff in the ``state == 0`` flow, 
            everything else has been copied from the ``rlcompleter`` std. 
            library module. 
            """ 
            if state == 0: 
                sys.path.insert(0, os.getcwd()) 
                # Calling python doesn't have a path, so add to sys.path. 
                try: 
                    logging.debug("Start REPL completion: " + repr(text))
                    interpreter = Interpreter(text, [namespace_module.__dict__]) 
 
                    lines = split_lines(text)
                    position = (len(lines), len(lines[-1]))
                    name = get_on_completion_name(
                        interpreter._module_node,
                        lines,
                        position
                    )
                    before = text[:len(text) - len(name)]
                    completions = interpreter.completions() 
                    logging.debug("REPL completions: %s", completions)
                except:
                    logging.error("REPL Completion error:\n" + traceback.format_exc())
                    raise
                finally: 
                    sys.path.pop(0) 
 
                self.matches = [before + c.name_with_symbols for c in completions] 
            try: 
                return self.matches[state] 
            except IndexError: 
                return None 
 
    try: 
        # Need to import this one as well to make sure it's executed before
        # this code. This didn't use to be an issue until 3.3. Starting with
        # 3.4 this is different, it always overwrites the completer if it's not
        # already imported here.
        import rlcompleter  # noqa: F401
        import readline 
    except ImportError: 
        print("Jedi: Module readline not available.")
    else: 
        readline.set_completer(JediRL().complete) 
        readline.parse_and_bind("tab: complete") 
        # jedi itself does the case matching 
        readline.parse_and_bind("set completion-ignore-case on") 
        # because it's easier to hit the tab just once 
        readline.parse_and_bind("set show-all-if-unmodified") 
        readline.parse_and_bind("set show-all-if-ambiguous on") 
        # don't repeat all the things written in the readline all the time 
        readline.parse_and_bind("set completion-prefix-display-length 2") 
        # No delimiters, Jedi handles that. 
        readline.set_completer_delims('') 
 
 
def version_info(): 
    """ 
    Returns a namedtuple of Jedi's version, similar to Python's 
    ``sys.version_info``. 
    """ 
    Version = namedtuple('Version', 'major, minor, micro') 
    from jedi import __version__ 
    tupl = re.findall(r'[a-z]+|\d+', __version__)
    return Version(*[x if i == 3 else int(x) for i, x in enumerate(tupl)]) 
