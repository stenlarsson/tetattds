#pragma once

#include <vcclr.h>
#include "udpconnectionmanager.h"
#include "servergame.h"

namespace server_gui {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;
	using namespace System::Runtime::InteropServices;

	/// <summary>
	/// Summary for MainForm
	///
	/// WARNING: If you change the name of this class, you will need to change the
	///          'Resource File Name' property for the managed resource compiler tool
	///          associated with all .resx files this class depends on.  Otherwise,
	///          the designers will not be able to interact properly with localized
	///          resources associated with this form.
	/// </summary>
	public ref class MainForm : public System::Windows::Forms::Form
	{
	public:
		MainForm(void)
		{
			InitializeComponent();
		}

		void AddLine(char* string)
		{
			AddLine(Marshal::PtrToStringAnsi((IntPtr)string));
		}

		void AddLine(String^ string)
		{
			historyBox->AppendText(string + "\r\n");
			historyBox->ScrollToCaret();
		}

	protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~MainForm()
		{
			delete connectionManager;
			delete game;

			if (components)
			{
				delete components;
			}
		}

		ServerGame* game;
		UdpConnectionManager* connectionManager;

	private: System::Windows::Forms::Timer^  pollTimer;

	private: System::Windows::Forms::Panel^  chatPanel;
	private: System::Windows::Forms::Button^  sendButton;
	private: System::Windows::Forms::TextBox^  chatTextBox;
	private: System::Windows::Forms::TextBox^  historyBox;
	private: System::Windows::Forms::NotifyIcon^  notifyIcon;
	private: System::Windows::Forms::ContextMenuStrip^  notifyIconMenu;
	private: System::Windows::Forms::ToolStripMenuItem^  openMenuItem;
	private: System::Windows::Forms::ToolStripMenuItem^  exitMenuItem;





	private: System::ComponentModel::IContainer^  components;
	protected: 

	protected: 

	private:
		/// <summary>
		/// Required designer variable.
		/// </summary>


#pragma region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		void InitializeComponent(void)
		{
			this->components = (gcnew System::ComponentModel::Container());
			System::ComponentModel::ComponentResourceManager^  resources = (gcnew System::ComponentModel::ComponentResourceManager(MainForm::typeid));
			this->pollTimer = (gcnew System::Windows::Forms::Timer(this->components));
			this->chatPanel = (gcnew System::Windows::Forms::Panel());
			this->sendButton = (gcnew System::Windows::Forms::Button());
			this->chatTextBox = (gcnew System::Windows::Forms::TextBox());
			this->historyBox = (gcnew System::Windows::Forms::TextBox());
			this->notifyIcon = (gcnew System::Windows::Forms::NotifyIcon(this->components));
			this->notifyIconMenu = (gcnew System::Windows::Forms::ContextMenuStrip(this->components));
			this->openMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->exitMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->chatPanel->SuspendLayout();
			this->notifyIconMenu->SuspendLayout();
			this->SuspendLayout();
			// 
			// pollTimer
			// 
			this->pollTimer->Enabled = true;
			this->pollTimer->Interval = 16;
			this->pollTimer->Tick += gcnew System::EventHandler(this, &MainForm::pollTimer_Tick);
			// 
			// chatPanel
			// 
			this->chatPanel->Controls->Add(this->sendButton);
			this->chatPanel->Controls->Add(this->chatTextBox);
			this->chatPanel->Dock = System::Windows::Forms::DockStyle::Bottom;
			this->chatPanel->Location = System::Drawing::Point(0, 240);
			this->chatPanel->Name = L"chatPanel";
			this->chatPanel->Size = System::Drawing::Size(292, 31);
			this->chatPanel->TabIndex = 1;
			// 
			// sendButton
			// 
			this->sendButton->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Bottom | System::Windows::Forms::AnchorStyles::Right));
			this->sendButton->Font = (gcnew System::Drawing::Font(L"Verdana", 8.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->sendButton->Location = System::Drawing::Point(214, 5);
			this->sendButton->Name = L"sendButton";
			this->sendButton->Size = System::Drawing::Size(75, 21);
			this->sendButton->TabIndex = 1;
			this->sendButton->Text = L"Send";
			this->sendButton->UseVisualStyleBackColor = true;
			this->sendButton->Click += gcnew System::EventHandler(this, &MainForm::sendButton_Click);
			// 
			// chatTextBox
			// 
			this->chatTextBox->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Bottom | System::Windows::Forms::AnchorStyles::Left) 
				| System::Windows::Forms::AnchorStyles::Right));
			this->chatTextBox->Font = (gcnew System::Drawing::Font(L"Verdana", 8.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->chatTextBox->Location = System::Drawing::Point(3, 5);
			this->chatTextBox->Name = L"chatTextBox";
			this->chatTextBox->Size = System::Drawing::Size(205, 21);
			this->chatTextBox->TabIndex = 0;
			// 
			// historyBox
			// 
			this->historyBox->BorderStyle = System::Windows::Forms::BorderStyle::None;
			this->historyBox->Dock = System::Windows::Forms::DockStyle::Fill;
			this->historyBox->Font = (gcnew System::Drawing::Font(L"Verdana", 8.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->historyBox->Location = System::Drawing::Point(0, 0);
			this->historyBox->Multiline = true;
			this->historyBox->Name = L"historyBox";
			this->historyBox->ScrollBars = System::Windows::Forms::ScrollBars::Vertical;
			this->historyBox->Size = System::Drawing::Size(292, 240);
			this->historyBox->TabIndex = 2;
			// 
			// notifyIcon
			// 
			this->notifyIcon->ContextMenuStrip = this->notifyIconMenu;
			this->notifyIcon->Icon = (cli::safe_cast<System::Drawing::Icon^  >(resources->GetObject(L"notifyIcon.Icon")));
			this->notifyIcon->Text = L"Tetattds Server 1.2";
			this->notifyIcon->Visible = true;
			this->notifyIcon->Click += gcnew System::EventHandler(this, &MainForm::notifyIcon_Click);
			// 
			// notifyIconMenu
			// 
			this->notifyIconMenu->Items->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(2) {this->openMenuItem, 
				this->exitMenuItem});
			this->notifyIconMenu->Name = L"notifyIconMenu";
			this->notifyIconMenu->Size = System::Drawing::Size(104, 48);
			// 
			// openMenuItem
			// 
			this->openMenuItem->Font = (gcnew System::Drawing::Font(L"Tahoma", 8.25F, System::Drawing::FontStyle::Bold));
			this->openMenuItem->Name = L"openMenuItem";
			this->openMenuItem->Size = System::Drawing::Size(103, 22);
			this->openMenuItem->Text = L"Open";
			this->openMenuItem->Click += gcnew System::EventHandler(this, &MainForm::openMenuItem_Click);
			// 
			// exitMenuItem
			// 
			this->exitMenuItem->Name = L"exitMenuItem";
			this->exitMenuItem->Size = System::Drawing::Size(103, 22);
			this->exitMenuItem->Text = L"Exit";
			this->exitMenuItem->Click += gcnew System::EventHandler(this, &MainForm::exitMenuItem_Click);
			// 
			// MainForm
			// 
			this->AcceptButton = this->sendButton;
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(292, 271);
			this->Controls->Add(this->historyBox);
			this->Controls->Add(this->chatPanel);
			this->Icon = (cli::safe_cast<System::Drawing::Icon^  >(resources->GetObject(L"$this.Icon")));
			this->Name = L"MainForm";
			this->Text = L"Tetattds Server";
			this->Resize += gcnew System::EventHandler(this, &MainForm::MainForm_Resize);
			this->Load += gcnew System::EventHandler(this, &MainForm::MainForm_Load);
			this->chatPanel->ResumeLayout(false);
			this->chatPanel->PerformLayout();
			this->notifyIconMenu->ResumeLayout(false);
			this->ResumeLayout(false);
			this->PerformLayout();

		}
#pragma endregion
	private: System::Void pollTimer_Tick(System::Object^  sender, System::EventArgs^  e) {
				connectionManager->Tick();
				game->Tick();
			 }

	private: System::Void sendButton_Click(System::Object^  sender, System::EventArgs^  e) {
				 chatTextBox->Focus();
				 String^ str = chatTextBox->Text->Trim();
				 if(str->Length == 0)
					 return;
				 chatTextBox->Text = "";
				 AddLine("Server: " + str);
				 pin_ptr<const wchar_t> wch = PtrToStringChars(str);
				 size_t convertedChars = 0;
				 size_t  sizeInBytes = ((str->Length + 1) * 2);
				 errno_t err = 0;
				 char    *ch = (char *)malloc(sizeInBytes);

				 err = wcstombs_s(&convertedChars,
					 ch, sizeInBytes,
					 wch, sizeInBytes);
				 if (err != 0)
					 throw "wcstombs_s  failed!";

				 game->SendChat(ch);
				 free(ch);
			 }
	private: System::Void MainForm_Resize(System::Object^  sender, System::EventArgs^  e) {
				 if(WindowState == FormWindowState::Minimized) {
					 Hide();
				 }
			 }
private: System::Void openMenuItem_Click(System::Object^  sender, System::EventArgs^  e) {
			 Show();
			 WindowState = FormWindowState::Normal;
		 }
private: System::Void exitMenuItem_Click(System::Object^  sender, System::EventArgs^  e) {
			 Close();
		 }
private: System::Void MainForm_Load(System::Object^  sender, System::EventArgs^  e) {
			 UdpSocket::InitSockets();

			 UdpSocket* socket = new UdpSocket();
			 if(!socket->Bind(13687)) {
				throw "Failed to bind socket";
			 }
			 game = new ServerGame();
			 connectionManager = new UdpConnectionManager(MAX_PLAYERS, socket, game);
		 }
private: System::Void notifyIcon_Click(System::Object^  sender, System::EventArgs^  e) {
			 Show();
			 WindowState = FormWindowState::Normal;
		 }
};
}

