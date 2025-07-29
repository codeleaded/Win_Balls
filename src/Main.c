// #include "C:/Wichtig/System/Static/Library/WindowEngine1.0.h"
// #include "C:/Wichtig/System/Static/Library/Random.h"
#include "/home/codeleaded/System/Static/Library/WindowEngine1.0.h"
#include "/home/codeleaded/System/Static/Library/Random.h"
#include "/home/codeleaded/System/Static/Library/TransformedView.h"

typedef struct Ball {
    Vec2 p;
    Vec2 v;
    F32 r;
    F32 m;
} Ball;

Ball Ball_New(Vec2 p,Vec2 v,F32 r,F32 m){
    Ball b;
    b.p = p;
    b.v = v;
    b.r = r;
    b.m = m;
    return b;
}
void Ball_Update(Ball* b,Rect border,float w->ElapsedTime){
    //if(b->p.x < -b->r)          b->p.x = 1.0f + b->r;
    //if(b->p.x > 1.0f + b->r)    b->p.x = 0.0f - b->r;
    //if(b->p.y < -b->r)          b->p.y = 1.0f + b->r;
    //if(b->p.y > 1.0f + b->r)    b->p.y = 0.0f - b->r;

    if(b->p.x < border.p.x + b->r){
        b->p.x = border.p.x + b->r;
        b->v.x *= -1.0f;
    }
    if(b->p.x > border.p.x + border.d.x - b->r){
        b->p.x = border.p.x + border.d.x - b->r;
        b->v.x *= -1.0f;
    }
    if(b->p.y < border.p.y + b->r){
        b->p.y = border.p.y + b->r;
        b->v.y *= -1.0f;
    }
    if(b->p.y > border.p.y + border.d.y - b->r){
        b->p.y = border.p.y + border.d.y - b->r;
        b->v.y *= -1.0f;
    }

    b->v = Vec2_Add(b->v,Vec2_Mulf(Vec2_Neg(b->v),0.1f * w->ElapsedTime));
    if(Vec2_Mag(b->v) < 0.0001f) b->v = (Vec2){ 0.0f,0.0f };
    
    b->p = Vec2_Add(b->p,Vec2_Mulf(b->v,w->ElapsedTime));
}
char Ball_Point(Ball* b,Vec2 p){
    float d = Vec2_Mag(Vec2_Sub(p,b->p));
    return d < b->r;
}
char Ball_Overlap(Ball* b1,Ball* b2){
    float d = Vec2_Mag(Vec2_Sub(b2->p,b1->p));
    return d < (b1->r + b2->r);
}
void Ball_ResolveColl(Ball* b1,Ball* b2){
    Vec2 d = Vec2_Sub(b1->p,b2->p);
	float h = Vec2_Mag(d);
	float Overlap = 0.5f * (h - b1->r - b2->r );
	b1->p = Vec2_Sub(b1->p,Vec2_Divf(Vec2_Mulf(d,Overlap),h));
	b2->p = Vec2_Add(b2->p,Vec2_Divf(Vec2_Mulf(d,Overlap),h));
	
	Vec2 n = Vec2_Divf(d,h);
	Vec2 t = Vec2_Perp(n);
	
	float d1 = Vec2_Dot(b1->v,t);
	float d2 = Vec2_Dot(b2->v,t);
	float dn1 = Vec2_Dot(b1->v,n);
	float dn2 = Vec2_Dot(b2->v,n);
	
	float m1 = (dn1 * (b1->m-b2->m) + 2.0f * b2->m * dn2) / (b1->m+b2->m);
	float m2 = (dn2 * (b2->m-b1->m) + 2.0f * b1->m * dn1) / (b1->m+b2->m);
	
	b1->v = Vec2_Add(Vec2_Mulf(t,d1),Vec2_Mulf(n,m1));
	b2->v = Vec2_Add(Vec2_Mulf(t,d2),Vec2_Mulf(n,m2));
}
void Ball_Render(Ball* b,TransformedView* tv){
    Vec2 p = TransformedView_WorldScreenPos(tv,b->p);
    float r = TransformedView_WorldScreenLX(tv,b->r);
    
    RenderCircleWire(p,r,WHITE,1.0f);
    
    Vec2 t = Vec2_Add(p,Vec2_Mulf(Vec2_Norm(b->v),r));
    RenderLine(p,t,BLUE,1.0f);
}
void Ball_Free(Ball* b){
    
}


int Selected = -1;
Vector Balls;
Rect Border;
TransformedView tv;

void Setup(AlxWindow* w){
    RGA_Set(Time_Nano());

    tv = TransformedView_New((Vec2){ GetHeight(),GetHeight() });
    Border = Rect_New((Vec2){ 0.0f,0.0f },(Vec2){ 5.0f,5.0f }); 
    Balls = Vector_New(sizeof(Ball));
    
    for(int i = 0;i<10;i++){
        float r = Random_f64_MinMax(0.1f,0.5f);
        Vector_Push(&Balls,(Ball[]){ Ball_New(
            (Vec2){ Random_f64_MinMax(Border.p.x,Border.p.x + Border.d.x),Random_f64_MinMax(Border.p.y,Border.p.y + Border.d.y) },
            (Vec2){ 0.0f,0.0f },
            r,
            r * 100.0f
        )});
    }
}
void Update(AlxWindow* w){
    TransformedView_HandlePanZoom(&tv,window.Strokes,GetMouse());

    Vec2 Mouse = TransformedView_ScreenWorldPos(&tv,GetMouse());

    if(Stroke(ALX_MOUSE_L).PRESSED){
        Selected = -1;
        for(int i = 0;i<Balls.size;i++){
            Ball* b = (Ball*)Vector_Get(&Balls,i);

            if(Ball_Point(b,Mouse)){
                Selected = i;
            }
        }
    }
    if(Stroke(ALX_MOUSE_L).RELEASED){
        if(Selected>=0){
            Ball* b = (Ball*)Vector_Get(&Balls,Selected);
            b->v = Vec2_Neg(Vec2_Sub(Mouse,b->p));
            Selected = -1;
        }
    }

    Clear(BLACK);

    Vec2 BorderP = TransformedView_WorldScreenPos(&tv,Border.p);
    Vec2 BorderD = TransformedView_WorldScreenLength(&tv,Border.d);
    RenderRectWire(BorderP.x,BorderP.y,BorderD.x,BorderD.y,WHITE,1.0f);
    
    for(int i = 0;i<Balls.size;i++){
        Ball* b1 = (Ball*)Vector_Get(&Balls,i);
        
        Ball_Update(b1,Border,w->ElapsedTime);

        for(int j = 0;j<Balls.size;j++){
            if(i==j) continue;
            
            Ball* b2 = (Ball*)Vector_Get(&Balls,j);
            if(Ball_Overlap(b1,b2)){
                Ball_ResolveColl(b1,b2);
                break;
            }
        }
        Ball_Render(b1,&tv);
    }

    if(Selected>=0){
        Ball* b = (Ball*)Vector_Get(&Balls,Selected);

        Vec2 p = TransformedView_WorldScreenPos(&tv,b->p);
        Vec2 t = GetMouse();
        RenderLine(p,t,RED,1.0f);
    }
}
void Delete(AlxWindow* w){
    for(int i = 0;i<Balls.size;i++){
        Ball* b = (Ball*)Vector_Get(&Balls,i);
        //Ball_Free(b);
    }
	Vector_Free(&Balls);
}

int main(){
    if(Create("Balls",1300,1300,1,1,Setup,Update,Delete))
        Start();
    return 0;
}