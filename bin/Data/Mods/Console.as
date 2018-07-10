/**
 * Registered console command structure
 * command - text that should be entered to call the event
 * eventToCall - name of the event that should be called when command is entered
 * description - short description of the command
 */
class ConsoleCommand {
    String command;
    String eventToCall;
    String description;
};

/**
 * List of all registered console commands
 */
Array<ConsoleCommand> consoleCommands = {};

Console@ console = engine.CreateConsole();

void Start()
{
    // No need to create console if the engine is in headless mode
    if (engine.headless) {
        return;
    }
    script.executeConsoleCommands = false;
    fileSystem.executeConsoleCommands = false;

    // Get default style
    XMLFile@ xmlFile = cache.GetResource("XMLFile", "UI/DefaultStyle.xml");
    if (xmlFile is null)
        return;

    console.defaultStyle = xmlFile;
    console.background.opacity = 0.8f;
    console.visible = false;
    console.numRows = graphics.height / 30;
    console.numBufferedRows = 20 * console.numRows;
    console.closeButton.visible = false;
    console.autoVisibleOnError = false;
    console.UpdateElements();
    console.commandInterpreter = "ScriptEventInvoker";
    log.timeStamp = false;
    log.level = 1;

    Subscribe();
}

void Stop()
{
    console.visible = false;
}

/**
 * Handle 'KeyDown' events
 */
void HandleKeyDown(StringHash eventType, VariantMap& eventData)
{
    int key = eventData["Key"].GetInt();

    // Toggle console by pressing F1
    if (key == KEY_F1) {
        console.Toggle();
    }
}

void Subscribe()
{
    /**
     * Event that is launched whenever user enters console command
     */
    SubscribeToEvent("ConsoleCommand", "HandleConsoleCommand");

    /**
     * Event that will allow all the components to register commands
     */
    SubscribeToEvent("ConsoleCommandAdd", "HandleConsoleCommandAdd");

    /**
     * Listener for the 'help' console command
     */
    VariantMap data;
    data["ConsoleCommandName"] = "help";
    data["ConsoleCommandEvent"] = "ConsoleHelp";
    data["ConsoleCommandDescription"] = "Displays all available commands";
    SendEvent("ConsoleCommandAdd", data);
    SubscribeToEvent("ConsoleHelp", "HandleConsoleHelp");

    /**
     * Listen for the 'KeyDown' events
     */
    SubscribeToEvent("KeyDown", "HandleKeyDown");

}

/**
 * Display information about all available console commands
 */
void HandleConsoleHelp(StringHash eventType, VariantMap& eventData)
{
    log.Info("");
    log.Info("###### All available (registered) commands ######");
    log.Info("#");
    for (uint i = 0; i < consoleCommands.length; i++) {
        log.Info("# '" + consoleCommands[i].command + ": " + consoleCommands[i].description + " (Event:" + consoleCommands[i].eventToCall + ")");
    }
    log.Info("#");
    log.Info("#########################");
    log.Info("");
}

/**
 * Add new console command
 */
void HandleConsoleCommandAdd(StringHash eventType, VariantMap& eventData)
{
    String command = eventData["ConsoleCommandName"].GetString();
    String eventToCall = eventData["ConsoleCommandEvent"].GetString();
    String description = eventData["ConsoleCommandDescription"].GetString();
    for (uint i = 0; i < consoleCommands.length; i++) {

        // If command already exists, throw an error.
        if (consoleCommands[i].command == command) {
            log.Error("Console command '" + command + "' already registered!");
            log.Error(command + " calls event '" + eventToCall + "'");
            return;
        }
    }

    // Register new console command
    ConsoleCommand consoleCommand;
    consoleCommand.command = command;
    consoleCommand.description = description;
    consoleCommand.eventToCall = eventToCall;
    consoleCommands.Push(consoleCommand);

    // Add command to autocomplete list
    if (!engine.headless) {
        console.AddAutoComplete(command);
    }
    log.Info("Console command '" + command + "' registered!");
}

/**
 * Whenever users input console command, process it
 */
void HandleConsoleCommand(StringHash eventType, VariantMap& eventData)
{
    // Safe guard to allow only Script events
    if (eventData["Id"].GetString() == "ScriptEventInvoker") {
        String inputValue = eventData["Command"].GetString();
        ParseCommand(inputValue);
    }
}

/**
 * Process the actual command and it's parameters
 */
void ParseCommand(String line)
{
    // All the parameters that are provided after the command will be passed
    // as a String array. 
    Array<String> commands = line.Split(' ', true);
    String command = commands[0];
    for (uint i = 0; i < consoleCommands.length; i++) {
        if (consoleCommands[i].command == command) {
            VariantMap data;
            data["Parameters"] = commands;

            // Call the actual event and pass all the parameters
            SendEvent(consoleCommands[i].eventToCall, data);
        }
    }
}