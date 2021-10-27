using System;

// =================================================================================================
// global variables
static public class Globals
{
	static public Renderer renderer;
	static public ArgHandler argHandler;
	static public ControlsHandler controlsHandler;
	static public ShaderHandler shaderHandler;
	static public VAOHandler vaoHandler;
	static public TextureHandler textureHandler;
	static public SoundHandler soundHandler;
	static public NetworkHandler networkHandler;
	static public PhysicsHandler physicsHandler;
	static public GameData gameData;
	static public GameItems gameItems;
	static public ActorHandler actorHandler;
	static public EffectHandler effectHandler;
	static public ScoreBoard scoreBoard;
	static public Random rand;

	static Globals() { }

	public static void Create (string[] args)
    {
		GL.Create();
		rand = new Random(Guid.NewGuid().GetHashCode());
		argHandler = new ArgHandler();
		argHandler.LoadArgs(args);
		argHandler.LoadArgs("smileybattle.ini");
		renderer = new Renderer(1920, 1080);
		textureHandler = new TextureHandler();
		vaoHandler = new VAOHandler();
		gameData = new GameData();
		actorHandler = new ActorHandler();
		gameItems = new GameItems();
		soundHandler = new SoundHandler();
		controlsHandler = new ControlsHandler();
		physicsHandler = new PhysicsHandler();
		networkHandler = new NetworkHandler();
		shaderHandler = new ShaderHandler();
		scoreBoard = new ScoreBoard();
		effectHandler = new EffectHandler();
		gameItems.Create();
		networkHandler.Create();
	}

}

// =================================================================================================
