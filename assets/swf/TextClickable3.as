/**
 * @author Harrie van Ginneken
 */
 
package  {

	import flash.display.MovieClip;	
	import flash.display.Stage;
	import flash.display.StageQuality;
	import flash.text.TextField;
	import flash.events.MouseEvent;
	import flash.events.Event;
	public class TextClickable3 extends MovieClip {
		//private var m_TextField : TextField;
		private var m_HitCount: Number;
		public var m_Hitbox_mc	: MovieClip;	
		public function TextClickable3() {
			trace ('Main')
			// stage.quality = StageQuality.BEST;
			// stage.scaleMode = "noScale";
			// stage.showDefaultContextMenu = false;
			// stage.align = "TL";
		m_HitCount = 0
		//m_TextField = this["target_text"];
		m_TextField.text = "Clicked Me " + m_HitCount.toString() + " times";
		//m_Hitbox_mc = this["mouse_hitbox"];
		CreateMouseButton();
		addEventListener( Event.ADDED_TO_STAGE, onLoad );
		}
	
		private function HandleRollOver(event: MouseEvent)
		{
			trace("TestClickable::HandleRollOver clicked: "+m_HitCount);
			m_TextField.textColor = 0x634634;
		}
		
		private function HandleRollOut(event: MouseEvent)
		{
			trace("TestClickable::HandleRollOut clicked: "+m_HitCount);
			m_TextField.textColor = 0xff0000;
		}
		
		private function HandleReleased(event: MouseEvent)
		{
			m_HitCount++;
			trace("TestClickable::HandleReleased clicked: "+m_HitCount);
			m_TextField.text = "Clicked Me " + m_HitCount + " times";
			m_TextField.textColor = 0x634634;
		}
		
		private function HandleMouseDown(event: MouseEvent)
		{
			trace("TestClickable::HandleMouseDown clicked: "+m_HitCount);
			m_TextField.textColor = 0x00FF00;
		}
		
		public function playAnimation(event: MouseEvent): void {
			trace("NUMPADBUTTON::playAnimation");
		}
	
		public function CreateMouseButton( ) 
		{
			trace("NUMPADBUTTON::CreateMouseButton");
			addEventListener(MouseEvent.MOUSE_DOWN, HandleMouseDown);
			addEventListener(MouseEvent.MOUSE_OVER, HandleRollOver);
			addEventListener(MouseEvent.MOUSE_OUT, HandleRollOut);
			addEventListener(MouseEvent.MOUSE_UP, HandleReleased);
			//m_Hitbox_mc.onRollOver = function (...innerArgs): void { HandleRollOut.apply(this, innerArgs.concat(args));};
			//Delegate.create(this, HandleRollOver);
			//m_Hitbox_mc.onRollOut  = Delegate.create(this, HandleRollOut);
			//m_Hitbox_mc.onRelease  = Delegate.create(this, HandleReleased);
			//m_Hitbox_mc.onMouseDown = Delegate.create(this, HandleMouseDown);
		}
		
		public function SetText( text : String)
		{
			//trace("Numpad Button SetText " + text);
			m_TextField.text = text;
		}
		
		public function onLoad(event: Event)
		{
			trace("TestClickable::onLoad" + event);
		}
		
	}
	
}