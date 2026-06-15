
function Game::CreateWallRect(%this, %edge, %class) {

    if (!RectIsValid(%edge)) {
        error("Game::CreateWallRect " SPC %class SPC "invalid Edge:" SPC %edge);
        return 0;
    }
    %bodyDef = new BodyDef2b() { type = $box2d::b2_staticBody; };
    %bodyDef.setPosition("0 0");

    if (%class $= "") %class = "WallRect";

    %wall = new Body2b() {
        class = %class;
    };
    %wall.CreateBody(%this.world,%bodyDef);
    %shape = new Shape2b();
    %shape.shapeType = $box2d::EdgeShape;
    %fixtureDef = new FixtureDef2b();
    %fixtureDef.setShape(%shape);

    %shape.setEdge(%edge);
    %wall.CreateFixture(%fixtureDef);

    %wall.visible = %wall.debugrender = %this.debugrender;
    //cleanup
    %bodyDef.delete();
    %shape.delete();
    %fixtureDef.delete();

    return wall;
}

