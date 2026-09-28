/* The host struct a module receives. Versioned; a module must check magic,
 * size and api before using anything, and must accept any api AT LEAST the
 * one it needs: fields are only ever appended, so a newer host is a superset.
 *   api 1: log only (size 28).
 *   api 2: register_event (size 32). Events fire at the game's own
 *          registration moments; a callback gets the host and an
 *          event-specific argument.
 *   api 3: the block API. Since B-034a these functions are PROVIDED by the
 *          core module (egg_core, on the SD card, pinned by CRC in the stage),
 *          so the loader's read-only budget no longer bounds the API. A host
 *          reports api 3 only when every api-3 provider is present, api 4
 *          only when every api-4 provider is present too.
 *            register_block  at EGG_EVENT_BLOCKS   (after Block::initBlocks;
 *                            the automatic block item then appears)
 *            bind_textures   at EGG_EVENT_GRAPHICS (argument is the parsed
 *                            blocks.json; the name must have an entry there)
 *            creative_add    at EGG_EVENT_CREATIVE (after each rebuild)
 *   api 4: virtual overrides and placement (size 64).
 *   api 5: teleport (size 68), an entity move by integer block delta.
 *   api 6: the attack event and its helpers (size 80).
 *            EGG_EVENT_ATTACK fires when the game is about to destroy a block for a
 *                            player: GameMode::destroyBlock, the one path every
 *                            destroy takes (creative instant break, survival
 *                            completion, wireless guests). The argument is a
 *                            const EggAttack *. A callback returning nonzero CANCELS
 *                            the destroy (the block stays, the game's destroyBlock
 *                            returns false); zero lets the game go on.
 *            held_item_id    the item id in the player's selected slot, 0 for none
 *            remove_block    sets a block to air through the game's own removeBlock
 *            attack_decode   core only: the stage hands it the hook's saved registers
 *                            (r0-r12, lr) and it fills the EggAttack; modules never
 *                            see a register
 *   api 7: shapes and the portal exit (size 112).
 *            block_shape     the render shape of a block id (the graphics record's
 *                            shape word; the renderer dispatches on it): EGG_SHAPE_*
 *                            below. Valid from EGG_EVENT_GRAPHICS on, after bind_textures.
 *            block_like      copies a stock block's render profile (render layer and
 *                            flags) onto a registered block: e.g. the ladder's for a
 *                            wall-flat block
 *            inherit_slot    points one virtual slot of a registered block at a stock
 *                            block's implementation (placement data, may-place, the
 *                            visual box...)
 *            get_block       the block id at a position in the player's world, -1 off;
 *                            EGG_BLOCK_SOLID is set on it when the game's solid table says so
 *            portal_exit     puts an entity one block in front of cell (x, y, z) on face
 *                            `out`, feet on the floor, turning its velocity and view by the
 *                            quarter turns from face `in` to face `out` (both are wall faces
 *                            2..5: the direction each portal opens toward)
 *            entity_cell     the block cell an entity's feet are in (x, y, z), from its box
 *            turn_entity     turns an entity's velocity and view as portal_exit would, without
 *                            moving it (the game has two player objects; the camera reads the
 *                            one the input paths hand to useOn and the attack event)
 *            entity_box      the entity's box, min xyz then max xyz, in sixteenths of a block
 *   api 8: a true item and the look direction (size 120).
 *            register_item   a plain Item (not a block item) with id 256..511, constructed by the
 *                            game's own Item constructor and put in the items table; call it once,
 *                            at the first EGG_EVENT_CREATIVE (after Item::initItems). Its icon is
 *                            the items-atlas slot named after it (the overlay's "items" assets).
 *                            Override its use (EGG_SLOT_ITEM_USE, returns the ItemInstance*) and
 *                            useOn (EGG_SLOT_ITEM_USE_ON, returns bool) on EGG_ITEM_VTABLE_BYTES;
 *                            creative_add lists it like a block item.
 *            entity_look     the entity's eye (sixteenths) and unit look direction (1/1024), for
 *                            a module's own ray march (yaw 0 = +Z, 90 = -X; pitch + = down)
 *   api 9: dimensions (size 128; docs/AETHER-DIMENSION-RE-2026-09-17.md section 8).
 *            register_dimension  a new dimension id (EGG_DIMENSION_FIRST..EGG_DIMENSION_LAST) with a
 *                            module-owned descriptor that must stay valid: the game's own
 *                            Level::createDimension builds every object for that id (the loader
 *                            patches it to take the id and the loader's vtable) as an overworld-class
 *                            dimension, and the core's copy of that vtable routes the spawn, the
 *                            arrival point and, when the descriptor has one, the terrain generator
 *                            to the descriptor. Call it at EGG_EVENT_BLOCKS; once per id.
 *            enter_dimension     move a player to a dimension by id (any id the game knows, 0 = the
 *                            overworld): creates the dimension on the player's level when absent and
 *                            asks the game's own change (Player::changeDimension), which finishes over
 *                            the next ticks. Refused when the player is already there, and for anything
 *                            that is not a player (5).
 *   api 10: arrivals and call-through (size 140; docs/B035b-AETHER-PORTAL-2026-09-18.md).
 *            enter_dimension_at  enter_dimension with the arrival chosen by the caller: the player lands
 *                            with its feet in block cell `cell` (x, y, z; centred) instead of the
 *                            destination's own rule (a registered dimension's spawn, the overworld's
 *                            identity for an origin past the End). Destinations: the overworld and
 *                            registered dimensions; refused (5) for the Nether and the End, whose
 *                            arrival the game computes itself, and from the End. The arrival is kept
 *                            per player until the game's change reads it (a plain enter_dimension
 *                            drops it), so a portal block may call it every tick like enter_dimension.
 *            slot_function   the function currently in a virtual slot of an object: the game's own
 *                            before an override, the override after. A module that overrides a stock
 *                            object's slot keeps this pointer and calls it for every case it does not
 *                            handle itself (call-through), so the module still carries no game address.
 *            instance_aux    the aux value (data) of an ItemInstance, -1 for none; the bucket's liquid
 *                            (8 water, 10 lava, 1 milk, 0 empty) lives here.
 *   api 11: variant families, velocity, friction, a collision slab (size 156; docs/B036-AETHER-PLAN).
 *            register_variants  the registered block is a family of `count` cubes on one id, told apart by
 *                            data 0..count-1: the tessellator picks the atlas slot by data (the overlay's
 *                            "variants" asset gives the block's texture key that many slots), and the core
 *                            answers the game's name and drop queries per data ("tile.<block>.<variant>.name",
 *                            the names given here, which must stay valid) and marks the block item as one with
 *                            subtypes at the creative event. Call it at EGG_EVENT_BLOCKS; list each variant with
 *                            creative_add(id, data).
 *            entity_velocity the entity's motion: EGG_VELOCITY_SET_Y sets the vertical speed (blocks per tick),
 *                            EGG_VELOCITY_DAMP_FALL multiplies a downward speed by the value (a cloud's slow fall)
 *            block_friction  the block's slipperiness (0.6 stone, 0.98 ice; quicksoil 1.1)
 *            block_collision a bottom slab of height/16 as the block's collision box (1..16), the full cell for
 *                            the crosshair; the block is not solid then (an aercloud's 0.01 box, walked on)
 *   api 12 (host size 176):
 *            allocate        `bytes` of zeroed memory from the game's heap, never freed (a generator's tables);
 *                            0 when the game refuses
 *            world_seed      the level's seed (32 bits) behind the LevelChunk handed to `generate`
 *            region_get      the block at a position through a BlockSource (`region`, as decorate is given):
 *                            id | data << 8 | EGG_REGION_SOLID, air where nothing is loaded
 *            region_set      sets id and data at a position through the BlockSource; nonzero = the game refused
 *            register_biome  a plains copy at a free id (EGG_BIOME_FIRST..EGG_BIOME_LAST) with the descriptor's sky
 *                            colour, rain and decoration hook; name it in a dimension's `biome`. Once per id, at
 *                            EGG_EVENT_BLOCKS.
 *   EggDimension (size >= 36): fog_rgb and a fixed time of day; EGG_DIMENSION_NO_STRUCTURES keeps the game's structures out.
 *   api 13 (host size 180):
 *            lookup_id       the registry's id for (EGG_KIND_BLOCK | EGG_KIND_ITEM, name), -1 when unregistered.
 *            register_block  / register_item now take EGG_ID_AUTO: the id is the registry's for that name; any
 *                            other id, or a name the registry lacks, is refused (0).
 *            override        copies the object's vtable into core-owned
 *                            storage on first use, points one slot at the
 *                            caller's own function, installs the copy
 *            make_passable   no collision, not solid (walk-through block)
 *            place_block     places block `id` at the targeted face through
 *                            that block's own item, so the game's placement
 *                            rules apply; for use from an item use override.
 *                            BlockItem::useOn on the console is
 *                            (ItemInstance&, Player&, const BlockPos& (3 ints),
 *                            signed char face, clicks in s0-s2): an override
 *                            declares (item, instance, player, const int *pos,
 *                            int face) and forwards pos[0..2] and face here
 *            item            the game's item object for an id, or NULL
 *            provide         core only: registers a provider function
 * Functions the game calls back (overrides) get their float arguments in VFP
 * registers: declare only the integer/pointer parameters and never touch
 * floating point in such a function, so they pass through untouched.
 */
#ifndef EGG_HOST_H
#define EGG_HOST_H
#include <stdint.h>
#define EGG_HOST_MAGIC 0x31474745u /* "EGG1" */
enum { EGG_EVENT_BLOCKS = 0, EGG_EVENT_GRAPHICS = 1, EGG_EVENT_CREATIVE = 2, EGG_EVENT_ATTACK = 3,
       EGG_EVENT_FACTORY = 4,      /* B-038a: after EntityFactory init (the end of every Level constructor): the creator map was just emptied */
       EGG_EVENT_RENDERERS = 5,    /* B-038a: after every rebuild of the entity renderer table (boot, resource pack changes) */
       EGG_EVENTS = 6 };
enum { EGG_PROVIDE_REGISTER_BLOCK = 0, EGG_PROVIDE_BIND_TEXTURES = 1, EGG_PROVIDE_CREATIVE_ADD = 2,
       EGG_PROVIDE_OVERRIDE = 3, EGG_PROVIDE_MAKE_PASSABLE = 4, EGG_PROVIDE_PLACE_BLOCK = 5, EGG_PROVIDE_ITEM = 6,
       EGG_PROVIDE_TELEPORT = 7, EGG_PROVIDE_HELD_ITEM = 8, EGG_PROVIDE_REMOVE_BLOCK = 9, EGG_PROVIDE_ATTACK_DECODE = 10,
       EGG_PROVIDE_BLOCK_SHAPE = 11, EGG_PROVIDE_BLOCK_LIKE = 12, EGG_PROVIDE_INHERIT_SLOT = 13, EGG_PROVIDE_GET_BLOCK = 14,
       EGG_PROVIDE_PORTAL_EXIT = 15, EGG_PROVIDE_ENTITY_CELL = 16, EGG_PROVIDE_TURN_ENTITY = 17, EGG_PROVIDE_ENTITY_BOX = 18,
       EGG_PROVIDE_REGISTER_ITEM = 19, EGG_PROVIDE_ENTITY_LOOK = 20, EGG_PROVIDE_REGISTER_DIMENSION = 21, EGG_PROVIDE_ENTER_DIMENSION = 22,
       EGG_PROVIDE_ENTER_DIMENSION_AT = 23, EGG_PROVIDE_SLOT_FUNCTION = 24, EGG_PROVIDE_INSTANCE_AUX = 25,
       EGG_PROVIDE_REGISTER_VARIANTS = 26, EGG_PROVIDE_ENTITY_VELOCITY = 27, EGG_PROVIDE_BLOCK_FRICTION = 28, EGG_PROVIDE_BLOCK_COLLISION = 29,
       EGG_PROVIDE_ALLOCATE = 30, EGG_PROVIDE_WORLD_SEED = 31, EGG_PROVIDE_REGION_GET = 32, EGG_PROVIDE_REGION_SET = 33, EGG_PROVIDE_REGISTER_BIOME = 34,
       EGG_PROVIDE_LOOKUP_ID = 35, EGG_PROVIDE_REGISTER_ENTITY = 36,
       EGG_PROVIDED = 37, EGG_PROVIDED_API3 = 3, EGG_PROVIDED_API4 = 7, EGG_PROVIDED_API5 = 8, EGG_PROVIDED_API6 = 11, EGG_PROVIDED_API7 = 19,
       EGG_PROVIDED_API8 = 21, EGG_PROVIDED_API9 = 23, EGG_PROVIDED_API10 = 26, EGG_PROVIDED_API11 = 30, EGG_PROVIDED_API12 = 35, EGG_PROVIDED_API13 = 36,
       EGG_PROVIDED_API14 = 37 };   /* api 15 (B-038a) changed register_entity's signature without adding a provider: a level of its own at the same count */
/* api 13: ids come from the loader's registry (the SD card's ids.bin, assigned by the host tool): a module registers a block
 * or an item by NAME with EGG_ID_AUTO and gets the registered object back; `lookup_id(kind, name)` tells the id (-1 = not in
 * the registry). A name the registry lacks is refused: nothing is guessed. Explicit ids are refused since api 13. */
enum { EGG_ID_AUTO = 0, EGG_KIND_BLOCK = 0, EGG_KIND_ITEM = 1, EGG_KIND_ENTITY = 2 };
/* B-038a: the stock mob a custom entity reuses (its C++ class, its type word's category bits, its rig): the game's type words. */
enum { EGG_ENTITY_CHICKEN = 0x130A, EGG_ENTITY_COW = 0x130B, EGG_ENTITY_PIG = 0x130C, EGG_ENTITY_SHEEP = 0x130D, EGG_ENTITY_RABBIT = 0x1312 };
enum { EGG_SPAWN_EGG = 383 };   /* the game's SpawnEggItem: Item::initItems hands its constructor 127 and Item::Item stores id + 256 (0x5790D8); item 127 is the cocoa block's */                              /* the game's spawn egg item; aux = the entity type's low byte */
enum { EGG_VELOCITY_SET_Y = 0, EGG_VELOCITY_DAMP_FALL = 1, EGG_VARIANTS_MAX = 16 };
/* Render shapes of USA v9.12.0's tessellator (88 cases; docs/B034d): a full cube, the crossed
 * quads of flowers and webs, the wall quad of the ladder (data 2..5 = the wall face it opens
 * toward), the multi-face quads of vines, and the thin flat quad of the lily pad. */
enum { EGG_SHAPE_CUBE = 0, EGG_SHAPE_CROSS = 1, EGG_SHAPE_LADDER = 8, EGG_SHAPE_VINE = 20, EGG_SHAPE_FLAT = 23 };
/* Stock block ids whose render profile is worth copying (never their virtuals: a stock method assumes
 * its own class). */
enum { EGG_BLOCK_LEAVES = 18, EGG_BLOCK_FLOWER = 37, EGG_BLOCK_WEB = 30, EGG_BLOCK_LADDER = 65, EGG_BLOCK_PORTAL = 90, EGG_BLOCK_VINE = 106, EGG_BLOCK_LILY = 111 };   /* the nether portal: the translucent pane profile */
enum { EGG_BLOCK_SOLID = 0x100, EGG_BLOCK_ID_MASK = 0xFF };
/* The attack event's argument: who is about to destroy which block, and the face they
 * targeted (0=-Y 1=+Y 2=-Z 3=+Z 4=-X 5=+X, -1 when the game gave none). */
typedef struct { void *player; int x, y, z; int face; } EggAttack;
/* Dimensions. The game's ids: 0 overworld, 1 nether, 2 the End, 3 its "undefined" sentinel; a
 * module's are 4..15. A chunk is 16 x 128 x 16: `generate` fills ids[column * 128 + y] with
 * column = x * 16 + z (block ids) and data[column * 64 + y / 2] (a nibble per block, the even y
 * in the low nibble); both arrive zeroed, so an untouched chunk is air. The descriptor's biome
 * (0 = the game's) is written to every column of a generated chunk. */
typedef struct EggHost EggHost;
typedef struct {
    uint32_t size;                                 /* sizeof(EggDimension) */
    uint32_t flags;                                /* EGG_DIMENSION_* below, 0 = none */
    int32_t spawn[3];                              /* the spawn point; also where an entering player lands (block coordinates) */
    uint32_t biome;                                /* biome id for generated chunks, 0 = leave the generator's */
    int (*generate)(const EggHost *host, void *chunk, int cx, int cz, uint8_t *ids, uint8_t *data);   /* 0 = the overworld's terrain */
    /* api 12 (size >= 36; older descriptors end above): */
    uint32_t fog_rgb;                              /* the fog colour, 0 = the overworld's (blue); the game scales it by daylight */
    int32_t time;                                  /* a fixed time of day in ticks (0..23999: the sun stands still there), -1 = the level's clock */
    /* B-036c (size >= 40; older descriptors end above): */
    int (*spawn_at)(const EggHost *host, uint32_t seed, int32_t out[3]);   /* the spawn and arrival point of a world, from its seed; 0 = written, else `spawn` above is used */
} EggDimension;
enum { EGG_DIMENSION_NO_STRUCTURES = 1 };           /* flags: the game's villages, mineshafts, strongholds, temples and dungeons stay out */
/* A biome registered at a free id (api 12): a copy of the plains biome whose sky colour, rain and decoration are the module's.
 * `decorate(region, cx, cz, seed)` runs when the game post-processes a generated chunk of that biome (the stage the game
 * decorates its own chunks at, after the 3 x 3 neighbourhood exists), with region_get/region_set for the blocks; the
 * block coordinates it may touch are chunk (cx, cz) and its neighbours. Flags: EGG_BIOME_NO_RAIN (no rain or snow falls),
 * EGG_BIOME_NO_LAKES (the game's own water lakes stay out: the copy reports the End biome's id to the game's lake test). */
typedef struct {
    uint32_t size;                                 /* sizeof(EggBiome) */
    uint32_t sky_rgb;                              /* the sky colour, 0 = the plains' (by temperature) */
    uint32_t flags;
    int (*decorate)(const EggHost *host, void *region, int cx, int cz, uint32_t seed);   /* 0 = no decoration at all */
} EggBiome;
enum { EGG_BIOME_NO_RAIN = 1, EGG_BIOME_NO_LAKES = 2, EGG_BIOME_FIRST = 40, EGG_BIOME_LAST = 127 };
enum { EGG_REGION_SOLID = 0x10000 };                /* region_get: id | data << 8, plus this when the game's solid table says so */
enum { EGG_DIMENSION_OVERWORLD = 0, EGG_DIMENSION_NETHER = 1, EGG_DIMENSION_END = 2, EGG_DIMENSION_FIRST = 4, EGG_DIMENSION_LAST = 15,
       EGG_CHUNK_IDS_BYTES = 16 * 16 * 128, EGG_CHUNK_DATA_BYTES = 16 * 16 * 64 };
/* Virtual slots (byte offsets into the vtable) and table sizes of USA v9.12.0,
 * read from the binary (docs/B034-PORTAL-RE-2026-09-16.md). */
enum { EGG_BLOCK_VTABLE_BYTES = 480, EGG_ITEM_VTABLE_BYTES = 352,
       EGG_SLOT_BLOCK_AABB = 0x20, EGG_SLOT_BLOCK_ENTITY_INSIDE = 0x118, EGG_SLOT_ITEM_USE_ON = 0x108,
       EGG_SLOT_BLOCK_MAY_PLACE = 0xBC, EGG_SLOT_BLOCK_PLACEMENT_DATA = 0x104, EGG_SLOT_BLOCK_VISUAL_SHAPE = 0x17C,
       EGG_SLOT_BLOCK_VISUAL_BOX = 0x180, EGG_SLOT_BLOCK_VARIANT = 0x184, EGG_SLOT_ITEM_USE = 0xA4 };   /* getVariant(data): the texture slot (base: the data itself) */
typedef int (*EggCallback)(const EggHost *host, void *argument);
typedef void (*EggFunction)(void);      /* any function, cast to and from its real type */
struct EggHost {
    uint32_t magic, size, api;
    void (*log)(const char *text);
    const char *name;
    uint32_t base, bytes;
    int (*register_event)(const EggHost *host, uint32_t event, EggCallback callback);
    /* api 3, provided (field order = EGG_PROVIDE_* order) */
    void *(*register_block)(const EggHost *host, const char *name, uint32_t id, float hardness, float resistance);
    int (*bind_textures)(const EggHost *host, void *json, const char *name);
    int (*creative_add)(const EggHost *host, uint32_t id, uint32_t aux);
    /* api 4, provided */
    int (*override)(const EggHost *host, void *object, uint32_t slot, EggFunction function, uint32_t vtable_bytes);
    int (*make_passable)(const EggHost *host, void *block);
    int (*place_block)(const EggHost *host, uint32_t id, void *player, int x, int y, int z, int face);
    void *(*item)(const EggHost *host, uint32_t id);
    /* api 5, provided: move an entity by an integer block delta. Translates every position
     * vector including the bounding box, so it survives the physics tick and preserves
     * velocity (mPos - mPosOld is unchanged). No game function is called. */
    int (*teleport)(const EggHost *host, void *entity, int dx, int dy, int dz);
    /* api 6, provided */
    int (*held_item_id)(const EggHost *host, void *player);
    int (*remove_block)(const EggHost *host, void *player, int x, int y, int z);
    int (*attack_decode)(const EggHost *host, const uint32_t *registers, EggAttack *attack);
    /* api 7, provided */
    int (*block_shape)(const EggHost *host, uint32_t id, uint32_t shape);
    int (*block_like)(const EggHost *host, void *block, uint32_t source_id);
    int (*inherit_slot)(const EggHost *host, void *block, uint32_t source_id, uint32_t slot);
    int (*get_block)(const EggHost *host, void *player, int x, int y, int z);
    int (*portal_exit)(const EggHost *host, void *entity, int x, int y, int z, int in, int out);
    int (*entity_cell)(const EggHost *host, void *entity, int *cell);
    int (*turn_entity)(const EggHost *host, void *entity, int in, int out);
    int (*entity_box)(const EggHost *host, void *entity, int *box16);
    /* api 8, provided */
    void *(*register_item)(const EggHost *host, const char *name, uint32_t id);
    int (*entity_look)(const EggHost *host, void *entity, int *eye16, int *dir1024);
    /* api 9, provided */
    int (*register_dimension)(const EggHost *host, uint32_t id, const EggDimension *dimension);
    int (*enter_dimension)(const EggHost *host, void *player, uint32_t id);
    /* api 10, provided */
    int (*enter_dimension_at)(const EggHost *host, void *player, uint32_t id, const int *cell);
    EggFunction (*slot_function)(const EggHost *host, void *object, uint32_t slot, uint32_t vtable_bytes);
    int (*instance_aux)(const EggHost *host, const void *instance);
    /* api 11, provided */
    int (*register_variants)(const EggHost *host, void *block, uint32_t count, const char *const *names);
    int (*entity_velocity)(const EggHost *host, void *entity, uint32_t op, float value);
    int (*block_friction)(const EggHost *host, void *block, float friction);
    int (*block_collision)(const EggHost *host, void *block, uint32_t height16);
    /* api 12, provided */
    void *(*allocate)(const EggHost *host, uint32_t bytes);
    int (*world_seed)(const EggHost *host, const void *chunk);
    int (*region_get)(const EggHost *host, void *region, int x, int y, int z);
    int (*region_set)(const EggHost *host, void *region, int x, int y, int z, uint32_t id, uint32_t data);
    int (*register_biome)(const EggHost *host, uint32_t id, const EggBiome *biome);
    /* api 13, provided */
    int (*lookup_id)(const EggHost *host, uint32_t kind, const char *name);
    /* api 14 (B-038a): a custom entity type. `name` is the registered name (kind EGG_KIND_ENTITY in the registry: the id is the type
     * word's low byte, the type word = base's category bits | id); `base` is one of EGG_ENTITY_*: the stock class whose constructor,
     * behaviours and rig the entity reuses. The overlay must carry server/entities/<name>.json (minecraft:<name>), the geometry
     * `geometry.<name>` in models/mobs.bjson and textures/entity/<name>.3dst. Call it at EGG_EVENT_BLOCKS; the core enters the game's
     * type registry then, re-enters the creator map at every EGG_EVENT_FACTORY and builds the renderer at every EGG_EVENT_RENDERERS.
     * Returns the id (the egg's aux: creative_add(host, EGG_SPAWN_EGG, id) lists it) or -1. */
    int (*register_entity)(const EggHost *host, const char *name, uint32_t id, uint32_t base, const char *texture);   /* api 15: `texture` = the pack texture name the entity renders with (a STOCK name whose file the overlay replaces, e.g. "textures/entity/camera_tripod"): the console loads no file the romfs does not list */
    /* stage, only the core module may call it */
    int (*provide)(const EggHost *host, uint32_t slot, EggFunction function);
};
#endif
