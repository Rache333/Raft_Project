import readline
import logging
import logic

LOG_FILENAME = '/tmp/completer.log'
logging.basicConfig(filename=LOG_FILENAME,
                    level=logging.DEBUG,
                    )

manual = '\nThis is a CLI for the RAFT LIKE project\nThe available commands are:\n1)show node status\n' \
         '2)show system status\n' \
         '3)log value [add,delete,edit] <key,value>\n' \
         '4)show log\n' \
         '5)exit\n\n' \
         'You can do only the specified commands or press exit to quit, press tab to complete cmd\n'


class Cli(object):

    def __init__(self, options):
        self.options = options
        self.current_candidates = []
        return

    def complete(self, text, state):
        response = None
        if state == 0:
            # This is the first time for this text, so build a match list.

            origline = readline.get_line_buffer()
            begin = readline.get_begidx()
            end = readline.get_endidx()
            being_completed = origline[begin:end]
            words = origline.split()

            logging.debug('origline=%s', repr(origline))
            logging.debug('begin=%s', begin)
            logging.debug('end=%s', end)
            logging.debug('being_completed=%s', being_completed)
            logging.debug('words=%s', words)

            if not words:
                self.current_candidates = sorted(self.options.keys())
            else:
                try:
                    if begin == 0:
                        # first word
                        candidates = self.options.keys()
                    else:
                        # later word
                        first = words[0]
                        candidates = self.options[first]

                    if being_completed:
                        # match options with portion of input
                        # being completed
                        self.current_candidates = [w for w in candidates
                                                   if w.startswith(being_completed)]
                    else:
                        # matching empty string so use all candidates
                        self.current_candidates = candidates

                    logging.debug('candidates=%s', self.current_candidates)

                except (KeyError, IndexError), err:
                    logging.error('completion error: %s', err)
                    self.current_candidates = []

        try:
            response = self.current_candidates[state]
        except IndexError:
            response = None
        logging.debug('complete(%s, %s) => %s', repr(text), state, response)
        return response

def input_loop():
    line = ''
    while line != 'exit':
        line = raw_input('>> ')

        if line == 'show node status':
            print "show_node_status()"

        elif line == 'show system status':
            print "show_system_status()"

        elif line == 'show log':
            print "show_log()"

        elif line.startswith("log value add"):
            line = line[14:]
            key, value = line.split(' ')
            if logic.editValToC(key,value) == -1:
                print("You need to have a Leader permission in order to execute the command")
            else:
                print ("you entered {0},{1} to the DataBase".format(key, value))

        elif line.startswith("log value delete"):
            line = line[17:]
            if logic.deleteValToC(line) == -1:
                print ("You need to have a Leader permission in order to execute the command")
            else:
                print ("you removed {0} from the DB".format(line))

        elif line.startswith("log value edit"):
            line = line[15:]
            key, value = line.split(' ')
            if logic.editValToC(key, value) == -1:
                print("You need to have a Leader permission in order to execute the command")
            else:
                print ("you changed the value of {0} to be {1}".format(key, value))

        elif line == 'help':
            print manual
        else:
            if not line == 'exit':
                print "unrecognized command: ", line


#Register our completer function
readline.set_completer(Cli(
    {'show': ['system status', 'node status', 'log'],
     'log value': ['add', 'delete', 'edit'],
     'exit': [],
     'help' : [],
     }).complete)


# Use the tab key for completion
readline.parse_and_bind('tab: complete')

# Prompt the user for text
input_loop()