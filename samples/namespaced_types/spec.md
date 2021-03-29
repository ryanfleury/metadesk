#namespace v1
@enum EntryKind : { A, B, C, E, }
@flags EntryFlags : { A, B, C, }

@union Color : {
    @struct components : {
        r : uint8_t;
        g : uint8_t;
        b : uint8_t;
        a : uint8_t;
    };
    raw : ([4]uint8_t);
}

@struct Entry : {
    to_remove : uint16_t;
    kind : EntryKind;
    flags : EntryFlags;
    color : Color;
    @struct p : {
        x : float;
        y : float;
    }
}

#namespace v2
@enum EntryKind : { A, B, B2, C, }
@flags EntryFlags : { B, C, D, }

@union Color : {
    // NOTE(mal): In the general case, it's impossible to decide automatically which member should be used 
    //            to port data across versions of a union. That's why I've introduced @authoritative here
    @authoritative 
    @struct components : {
        b : uint8_t;
        g : uint8_t;
        r : uint8_t;
        a : uint8_t;
    }
    raw : ([4]uint8_t);
}

@struct Entry : {
    kind : EntryKind;
    color : Color;
    @struct p : {
        x : float;
        y : float;
        z : float;
    };
    flags : EntryFlags;
}
