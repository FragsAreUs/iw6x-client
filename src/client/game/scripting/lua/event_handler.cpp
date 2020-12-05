#include "std_include.hpp"
#include "context.hpp"
#include "error.hpp"
#include "value_conversion.hpp"

namespace scripting::lua
{
	event_handler::event_handler(sol::state& state)
		: state_(state)
	{
		auto event_listener_handle_type = state.new_usertype<event_listener_handle>("event_listener_handle");

		event_listener_handle_type["clear"] = [this](const event_listener_handle& handle)
		{
			this->remove(handle);
		};
	}

	void event_handler::dispatch(const event& event)
	{
		std::vector<sol::lua_value> arguments;

		for (const auto& argument : event.arguments)
		{
			arguments.emplace_back(convert(this->state_, argument));
		}

		this->dispatch_to_specific_listeners(event, arguments);
	}

	void event_handler::dispatch_to_specific_listeners(const event& event,
	                                                   const event_arguments& arguments)
	{
		for (auto listener : this->event_listeners_)
		{
			if (listener->event == event.name && listener->entity_id == event.entity.get_entity_id())
			{
				if (listener->is_volatile)
				{
					this->event_listeners_.remove(listener);
				}

				try
				{
					listener->callback(arguments);
				}
				catch (std::exception& e)
				{
					handle_error(e);
				}
			}
		}
	}

	event_listener_handle event_handler::add_event_listener(event_listener&& listener)
	{
		listener.id = ++this->current_listener_id_;
		this->event_listeners_.add(listener);
		return {listener.id};
	}

	void event_handler::clear()
	{
		this->event_listeners_.clear();
	}

	void event_handler::remove(const event_listener_handle& handle)
	{
		for (const auto task : this->event_listeners_)
		{
			if (task->id == handle.id)
			{
				this->event_listeners_.remove(task);
				return;
			}
		}
	}
}