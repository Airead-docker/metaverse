/**
 * Copyright (c) 2016-2017 mvs developers 
 *
 * This file is part of metaverse-explorer.
 *
 * metaverse-explorer is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Affero General Public License with
 * additional permissions to the one published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version. For more information see LICENSE.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <boost/property_tree/ptree.hpp>      
#include <boost/property_tree/json_parser.hpp>

#include <metaverse/bitcoin.hpp>
#include <metaverse/client.hpp>
#include <metaverse/explorer/define.hpp>
#include <metaverse/explorer/callback_state.hpp>
#include <metaverse/explorer/display.hpp>
#include <metaverse/explorer/prop_tree.hpp>
#include <metaverse/explorer/dispatch.hpp>
#include <metaverse/explorer/extensions/wallet/createasset.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/command_assistant.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {

namespace pt = boost::property_tree;

#define IN_DEVELOPING "this command is in deliberation, or replace it with original command."
/************************ createasset *************************/

console_result createasset::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    blockchain.is_account_passwd_valid(auth_.name, auth_.auth);
    // maybe throw
    blockchain.uppercase_symbol(option_.symbol);
    
    auto ret = blockchain.is_asset_exist(option_.symbol);
    if(ret) 
        throw std::logic_error{"asset symbol is already exist, please use another one"};
    if (option_.symbol.length() > ASSET_DETAIL_SYMBOL_FIX_SIZE)
        throw std::logic_error{"asset symbol length must be less than 64."};
    if (option_.address.length() > ASSET_DETAIL_ADDRESS_FIX_SIZE)
        throw std::logic_error{"asset address length must be less than 64."};
    if (!option_.address.empty() && !blockchain.is_valid_address(option_.address))
        throw std::logic_error{"invalid address parameter!"};
    if (option_.description.length() > ASSET_DETAIL_DESCRIPTION_FIX_SIZE)
        throw std::logic_error{"asset description length must be less than 64."};
    if (auth_.name.length() > 64) // maybe will be remove later
        throw std::logic_error{"asset issue(account name) length must be less than 64."};

    auto acc = std::make_shared<asset_detail>();
    acc->set_symbol(option_.symbol);
    acc->set_maximum_supply(option_.maximum_supply);
    //acc->set_asset_type(option_.asset_type); // todo -- type not defined
    acc->set_asset_type(asset_detail::asset_detail_type::created); 
    acc->set_issuer(auth_.name);
    acc->set_address(option_.address);
    acc->set_description(option_.description);
    
    blockchain.store_account_asset(acc);

    //output<<option_.symbol<<" created at local, you can issue it.";
    
    pt::ptree aroot;
    pt::ptree asset_data;
    asset_data.put("symbol", acc->get_symbol());
    asset_data.put("maximum_supply", acc->get_maximum_supply());
    asset_data.put("asset_type", acc->get_asset_type());
    asset_data.put("issuer", acc->get_issuer());
    asset_data.put("address", acc->get_address());
    asset_data.put("description", acc->get_description());
    //asset_data.put("status", "issued");
    aroot.push_back(std::make_pair("asset", asset_data));
        
    pt::write_json(output, aroot);
    
    return console_result::okay;
}


} // namespace commands
} // namespace explorer
} // namespace libbitcoin
